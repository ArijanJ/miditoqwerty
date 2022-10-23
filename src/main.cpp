#include <iostream>
#include <cstdlib>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#undef WinMain // <- because SDL_main has its own main/WinMain

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "themes.h"

#include "Audio.h"
#include "Log.h"
#include "Piano.h"
#include "Midi.h"

#include "util.h"
#include "settings.h"
#include "Qwerty.h"
#include "inpututils.h" // do loadCharsets in main

#include "GL/gl3w.h"            // Initialize with gl3wInit()
#include <sstream>

#define POSSIBLYEDITABLE (ImGuiWindowFlags_NoBringToFrontOnFocus | (!windowsEditable ? \
                                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | \
                                             ImGuiWindowFlags_NoCollapse \
                                             : \
                                             ImGuiWindowFlags_None))

#define CONTROL_CHANGE (status >= 0xB0 && status <= 0xBF)
#define NOTE_ON        (status >= 0x90 && status <= 0x9F)
#define NOTE_OFF       (status >= 0x80 && status <= 0x8F)

// i am absolutely mental
void (*dyn_sendKeyDown)(char); //~ = sendKeyDown etc.
void (*dyn_sendKeyUp)(char, char);
void (*dyn_setVelocity)(char);
void (*dyn_sendOutOfRangeKey)(char);

// Settings

// bad extern lol what am i doing
int logStuff;

// state
bool resetting = false;

bool sustainOn = false;
bool ctrlDown = false;
bool rightDown = false;

int scanSetChoice = 0;

// saved in settings
std::string defaultFont;
std::string defaultTheme;// = "themes/default.theme";

// initialize externs
std::string currentFont;
std::string currentTheme;

ImVec4 gBackgroundColor; // not settings, theme props
ImVec4 gNoteColor;
// removed static cause extern
ImVec4 gNoteNameColor; // load this

static int showTitlebar = 1;
static int windowOpacity = 100;
static int windowsEditable = 0;

static int smallLayout = 0;

static int alwaysontop = 1;

static int qwertyEmulator = 1;

static int enableOutput = 1;
static int eightyeightkey = 1;
static int sustain = 1;
static int velocity = 1;

int sustainCutoff = 64; // Inclusive

SDL_Window* window;

SettingsHandler settingsHandler;

Piano piano;

Midi midi;

PmTimestamp lastNotePlayed = 0;
Audio audio;

Log logger;

// could be worse tbh
void setEmulatorFunctions() {
    if (qwertyEmulator == 0) {
        dyn_sendKeyDown = sendKeyDown;
        dyn_sendKeyUp = sendKeyUp;
        dyn_setVelocity = setVelocity;
        dyn_sendOutOfRangeKey = sendOutOfRangeKey;
    }
    else if (qwertyEmulator > 0) {
        dyn_sendKeyDown = qwerty_sendKeyDown;
        dyn_sendKeyUp = qwerty_sendKeyUp;
        dyn_setVelocity = qwerty_setVelocity;
        dyn_sendOutOfRangeKey = qwerty_sendOutOfRangeKey;
        if (qwertyEmulator == 1)
            scanSetChoice = 0;
        else if (qwertyEmulator == 2)
            scanSetChoice = 1;
        else if (qwertyEmulator == 3)
            scanSetChoice = 2;
    }

    // ..?

    printf("Routing emulator functions complete, emulator mode %d with scanset %d\n", qwertyEmulator, scanSetChoice);
}

void refreshSettings(){
    ImGui::LoadIniSettingsFromDisk((smallLayout ? "layout_small.ini" : "layout_tall.ini"));
    SDL_SetWindowOpacity(window, (float)windowOpacity / 100);
    SDL_SetWindowBordered(window, (showTitlebar ? SDL_TRUE : SDL_FALSE));
    SDL_SetWindowAlwaysOnTop(window, (alwaysontop ? SDL_TRUE : SDL_FALSE));
    setEmulatorFunctions();
}

void resetSettings() {
    
    alwaysontop = true;

    enableOutput = true;

    eightyeightkey = true;
    sustain = true;
    velocity = true;

    sustainCutoff = 64;

    showTitlebar = true;
    windowOpacity = 100;
    windowsEditable = false;

    currentTheme = "default";
    defaultTheme = "themes/default.theme";
    logStuff = true;
    LoadTheme(defaultTheme);
    refreshSettings();

    settingsHandler.DumpSettings();
}

void pollCallback(PmTimestamp timestamp, uint8_t status, PmMessage Data1, PmMessage Data2); // forwards
void AdvanceImGuiFrame();

// Main code
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    //AllocConsole();
    //ShowWindow(GetConsoleWindow(), SW_HIDE); // hide on startup
    FILE* stdoutNew;
    freopen_s(&stdoutNew, "log.txt", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);

    loadScansets(); // for input

    settingsHandler.AddSetting("Always on top", &alwaysontop);
    settingsHandler.AddSetting("Editable windows", &windowsEditable);
    settingsHandler.AddSetting("Show titlebar", &showTitlebar);
    settingsHandler.AddSetting("Window opacity", &windowOpacity);
    settingsHandler.AddSetting("Small layout", &smallLayout);
    settingsHandler.AddSetting("Enable output", &enableOutput);
    settingsHandler.AddSetting("88-key support", &eightyeightkey);
    settingsHandler.AddSetting("Sustain", &sustain);
    settingsHandler.AddSetting("Velocity", &velocity);
    settingsHandler.AddSetting("Sustain cutoff", &sustainCutoff);
    settingsHandler.AddSetting("Log stuff", &logStuff);
    settingsHandler.AddSetting("QWERTY emulation", &qwertyEmulator);

    fflush(stdoutNew);

    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL        |
                                                     SDL_WINDOW_ALLOW_HIGHDPI |
                                                     SDL_WINDOW_ALWAYS_ON_TOP );
    window = SDL_CreateWindow("Midi to Qwerty", SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, 435, 550, window_flags);
    //SDL_SetWindowHitTest(window, DragCallback, 0); - old way of dragging with ctrl
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    bool err = gl3wInit() != 0;
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL; // Manual setting with LoadIniFileXXX
    ImGui::LoadIniSettingsFromDisk((smallLayout ? "layout_small.ini" : "layout_tall.ini"));

    std::string initializedFont;

    fflush(stdoutNew);

    // Load fonts 
    for (auto& p : std::filesystem::recursive_directory_iterator("fonts")) // Add the rest of the fonts in fonts/
    {
        if (p.path().extension() == ".ttf") {

            std::string relativePath = p.path().stem().string();
            if (relativePath == defaultFont) continue; // Dont load default font, already loaded

            std::string fullPath = "fonts/" + relativePath + ".ttf";

            if (fullPath == initializedFont) continue;

            printf("Font found: %s\n", fullPath.c_str());

            ImGui::GetIO().Fonts->AddFontFromFileTTF(fullPath.c_str(), 13);
        }
    }

    fflush(stdoutNew);

    (void) io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowTitleAlign = ImVec2(0.5f, 0.5f);
    ImGui::GetStyle().WindowRounding = 8.0f;
    ImGui::GetStyle().FrameRounding = 4.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // if couldnt load settings
    if (!settingsHandler.LoadSettings()) {
        printf("Could not load settings - resetting them\n");
        resetSettings();
        settingsHandler.DumpSettings();
    }

    if (smallLayout) {
        SDL_SetWindowSize(window, 435, 315);
        printf("Loaded small layout\n");
    }
    else {
        SDL_SetWindowSize(window, 435, 550);
        printf("Loaded tall layout\n");
    }

    // Some settings need to be applied before the main loop because they rely on immediate mode paradigm
    refreshSettings();
    
    int selectedDevice = -1;
     
    //Settings settings;
    bool show_midi_window = true;
    bool show_piano_window = true;
    bool show_log_window = true;
    bool show_settings = true;
    bool rainbowMode = false;
    
    // only global used ImVec4 clear_color = gBackgroundColor;//ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

    // Main loop
    bool done = false;
    while (!done) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_LCTRL)
                    ctrlDown = true;
                else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    done = true;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    rightDown = true;
                }
            }
            if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    rightDown = false;
                }
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        if (selectedDevice == -1) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);

            //ImFont* pcurrfont = ImGui::GetFont();
            //ImFont bigmode = ImFont(*pcurrfont);
            //bigmode.Scale = 1.5f;
            //ImGui::PushFont(&bigmode);

            ImGui::Begin("Input selection", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);

            static int currindex = 0;
            ImGui::SetNextItemWidth(viewport->WorkSize.x);
            ImGui::BeginListBox("###input", {-1, -1});

            static std::vector<MidiDevice> devices = MidiUtils::GetDevices();

            if (devices.size() < 1) {
                ImGui::Selectable("You don't seem to have any MIDI devices");
                continue;
            }

            static bool loaded = false;
            int ninputs = 0;

            for (auto& device : devices)
                if (device.input)
                    ninputs++;

            if (ninputs == 1) {
                for (auto& device : devices) {
                    if (device.input) {
                        selectedDevice = midi.deviceID = device.id;
                        midi.InitWrapper();
                        loaded = true;
                        logger.AddLog("Opening the only MIDI input");
                    }
                }
            }

            if (loaded == true) continue;

            for (const MidiDevice &device : devices) {

                std::string tooltip = "";

                if (!device.input) { // Color them red or something and add a hint to why they're not okay
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                    tooltip = "This isn't an input MIDI device.";
                }

                std::string displayString = device.name;
                if (device.isdefault) displayString += " [default]";
                if (ImGui::Selectable(displayString.c_str()) && tooltip == "") // if all is good
                {
                    selectedDevice = midi.deviceID = device.id;
                    midi.InitWrapper(); //
                    //ImGui::PopFont();
                }
                if (ImGui::IsItemHovered() && tooltip != "")
                    ImGui::SetTooltip(tooltip.c_str());
                ImGui::PopStyleVar();

            }

            ImGui::EndListBox();

            ImGui::End();

            // Rendering
            AdvanceImGuiFrame();
            continue;
        }

        if (show_midi_window) {
            ImGui::Begin("Midi", NULL, POSSIBLYEDITABLE);
            std::ostringstream os;
            auto current_notes = piano.current_notes();
            for (auto &note : current_notes) {
                os << midiNoteString(note) << " ";
            }
            std::string playing = os.str();
            ImGui::Text("Playing: ");
            ImGui::SameLine();
            if (!playing.empty()) {
                ImGui::TextColored(gNoteNameColor, "%s", playing.c_str());
            } else {
                ImGui::Text("");
            }
            std::string chord = midiChordString(current_notes);
            ImGui::Text("Qwerty: ");
            ImGui::SameLine();
            if (!playing.empty()) {
                for (auto& note : current_notes) {
                    //logger.AddLog("NOTE %c", note);
                    //ImGui::TextColored(ImVec4(0, 1, 0, 1), "%d", note);
                    if (note > 0 && note < 36) {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%c ", lowNotes.c_str()[abs(note - 35)]);
                    }
                    // Okay
                    else if (note >= 36 && note <= 96) {
                        ImGui::TextColored(ImVec4(0, 0, 1, 1), "%c ", letterNoteMap.c_str()[note - 36]);;
                    }
                    // High
                    else if (note > 96 && note < 122) { 
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%c ", highNotes.c_str()[note - 97]);
                    }
                    else {
                        logger.AddLog("Could not find key %d\n", note);
                    }
                    ImGui::SameLine();
                }

            } else { ImGui::Text(""); }
            ImGui::End();
        }

        midi.poll(pollCallback, true);

        if (rainbowMode) { /*Do all the rainbow stuff in one block*/
            static int r = 0; static int g = 0; static int b = 0;

            ImVec4 normalizedRainbow = ImVec4((float)r / 255, (float)g / 255, (float)b / 255, 1.0f);

            advanceRainbow(&r, &b, &g);

            gBackgroundColor    = normalizedRainbow;
            gNoteColor          = normalizedRainbow;
            gNoteNameColor      = normalizedRainbow;
        }

        if (show_piano_window) {
            piano.draw(&show_piano_window, windowsEditable,
                IM_COL32(gNoteColor.x * 255, gNoteColor.y * 255, gNoteColor.z * 255, 255));
        }

        if (true) {
            ImGui::Begin("Settings", NULL, POSSIBLYEDITABLE);

            ImGui::Text("Window settings");
            if (ImGui::Checkbox("Always on top", (bool*)&alwaysontop))
                SDL_SetWindowAlwaysOnTop(window, (SDL_bool)alwaysontop);
            ImGui::Checkbox("Editable windows", (bool*)& windowsEditable);
            if (ImGui::Checkbox("Show titlebar", (bool*)&showTitlebar)) {
                // do below SDL_SetWindowOpacity(window, 0.0f);
                SDL_SetWindowBordered(window, (SDL_bool)showTitlebar);
            }

            ImGui::Text("Opacity");
            if (ImGui::SliderInt("##", &windowOpacity, 10, 100, "%d%%")) { 
                logger.AddLog("Setting opacity to %d\n", windowOpacity);
                SDL_SetWindowOpacity(window, (float)windowOpacity / 100);
            }

            static bool showStyleEditor = false;
            if (ImGui::Button((!showStyleEditor ? "Open theme editor" : "Close theme editor")))
                showStyleEditor = !showStyleEditor;
            

            const char* layouts[] = { "Small", "Tall" };
            static const char* current_item = (smallLayout?"Small":"Tall");

            //ImGui::GetStyle().

            ImGui::Text("Layout");
            ImGui::PushItemWidth(ImGui::GetFontSize() * 6); // 6 chars - Small, Tall = 5, 4
            if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
            {
                for (int n = 0; n < IM_ARRAYSIZE(layouts); n++)
                {
                    bool is_selected = (current_item == layouts[n]); // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(layouts[n], is_selected)) {
                        current_item = layouts[n];
                        if (current_item == "Small") {
                            SDL_SetWindowSize(window, 435, 310);
                            ImGui::LoadIniSettingsFromDisk("layout_small.ini");
                            smallLayout = true;
                        }
                        else if (current_item == "Tall") {
                            SDL_SetWindowSize(window, 435, 550);
                            ImGui::LoadIniSettingsFromDisk("layout_tall.ini");
                            smallLayout = false;
                        }
                    }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();

            if (showStyleEditor) {
                ImGui::Begin("Theme Editor");
                
                ImGui::ShowStyleEditor();
                ImGui::End();
            }

            ImGui::ColorEdit3("Background color", (float*)&gBackgroundColor, ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit3("Note color", (float*)&gNoteColor, ImGuiColorEditFlags_NoInputs);

            ImGui::Checkbox("RAINBOW MODE!", &rainbowMode);

            ImGui::Text("Piano settings");

            if (ImGui::Checkbox("Enable output", (bool*)&enableOutput)) {
                // Clear all notes
                for (int i = 21; i <= 108; i++)
                    piano.up(i);
            }

            ImGui::Checkbox("88-key support", (bool*)& eightyeightkey);

            ImGui::Checkbox("Sustain", (bool*)& sustain);

            ImGui::Text("Sustain cutoff");
            ImGui::SliderInt("", &sustainCutoff, 0, 127);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("CTRL + Click to enter a value,\ndefault is 64");
            }
   
            ImGui::Checkbox("Velocity", (bool*)& velocity);

            ImGui::Text("QWERTY Emulator");
            static const char* qwertyEmulatorMode;
            static bool didEmuTextInit = false;
            if (!didEmuTextInit) {
                if (qwertyEmulator == 0) qwertyEmulatorMode = "Off";
                if (qwertyEmulator == 1) qwertyEmulatorMode = "Set 1";
                if (qwertyEmulator == 2) qwertyEmulatorMode = "Set 2";
                didEmuTextInit = true;
            } // these save

            if (ImGui::BeginCombo("Mode", qwertyEmulatorMode)) {
                if (ImGui::Selectable("Off")) {
                    qwertyEmulatorMode = "Off";
                    qwertyEmulator = 0;
                    setEmulatorFunctions();
                }
                if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Consults Windows and your keyboard layout,\nuseful for playing outside of Roblox");

                if (ImGui::Selectable("Set 1")) {
                    qwertyEmulatorMode = "Set 1";
                    qwertyEmulator = 1;
                    setEmulatorFunctions();
                }
                if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("This should be your go-to setting for Roblox");

                if (ImGui::Selectable("Set 2")) {
                    qwertyEmulatorMode = "Set 2";
                    qwertyEmulator = 2;
                    setEmulatorFunctions();
                }
                if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Use this if your keyboard doesn't\nproperly support Set 1");

                if (ImGui::Selectable("QWERTZ")) {
                    qwertyEmulatorMode = "QWERTZ";
                    qwertyEmulator = 3;
                    setEmulatorFunctions();
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("This is like Set 1, but swaps Y with Z");

                ImGui::EndCombo();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Allows you to play on alternate keyboard layouts,\n should be faster in practice for normal use");
            }

            ImGui::Text("MIDI Input");
            static const char* selectedDeviceName = Pm_GetDeviceInfo(selectedDevice)->name;
            if (ImGui::BeginCombo("Port", selectedDeviceName)) {
                for (int i = 0; i < Pm_CountDevices() - 1; i++) {
                    const PmDeviceInfo* deviceInfo = Pm_GetDeviceInfo(i);
                    if (deviceInfo->input == 0) continue;
                    bool is_selected = (selectedDeviceName == Pm_GetDeviceInfo(selectedDevice)->name);

                    if (ImGui::Selectable(deviceInfo->name, is_selected)) {
                        std::cout << "Changed to " << deviceInfo->name << '\n';
                        selectedDeviceName = deviceInfo->name;
                        midi.shutdown(midi.stream);
                        midi.deviceID = i;
                        midi.InitWrapper();  // ......... if it works it works right?
                        logger.AddLog("Opened MIDI device %s\n", selectedDeviceName);
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Text("Reset");

            if (ImGui::Button("Reset settings")) {
                resetting = true;
            }

            if (resetting) {
                ImGui::PushItemWidth(ImGui::GetFontSize() * 7.0f);
                ImGui::SameLine();
                if (ImGui::Button("Yes")) {
                    printf("Resetting settings\n");
                    resetSettings();
                    resetting = false; 
                }
                ImGui::SameLine();
                if (ImGui::Button("No..."))
                    resetting = false;
            }

            ImGui::End();
        }

        if (true) { // LOG(ger) WINDOW
            ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
            //ImGui::PushItemWidth(ImGui::GetFontSize() * 10); not here
            ImGui::Begin("Log", NULL, POSSIBLYEDITABLE);
            //ImGui::PopItemWidth();
            ImGui::End();

            // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
            logger.Draw("Log", &show_log_window);
        }
        AdvanceImGuiFrame();
    }

    settingsHandler.DumpSettings();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void AdvanceImGuiFrame() {
    ImGuiIO& io = ImGui::GetIO();

    // Move window with right click
    static bool firstDown = true;
    static int relOldMousePos[2];
    if (rightDown) {
        int x, y;
        int winx, winy;
        int sizex, sizey;
        SDL_GetWindowPosition(window, &winx, &winy);
        SDL_GetWindowSize(window, &sizex, &sizey);
        SDL_GetMouseState(&x, &y);
        x -= sizex / 2;
        y -= sizey / 2;
        if (firstDown) {
            SDL_GetMouseState(&relOldMousePos[0], &relOldMousePos[1]);
            firstDown = false;
        }
        //SDL_SetRelativeMouseMode(SDL_TRUE);
        SDL_SetWindowPosition(window, winx + x, winy + y);
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(gBackgroundColor.x, gBackgroundColor.y, gBackgroundColor.z, gBackgroundColor.w); // w was here
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
}

void pollCallback(PmTimestamp timestamp, uint8_t status, PmMessage Data1, PmMessage Data2) {
    logger.AddLog("Event status: %d, Data1: %04X, Data2: %04X\n", status, Data1, Data2);

    if (!enableOutput) {
        if (NOTE_ON)
            piano.down(Data1, Data2);
        else if (NOTE_OFF) {
            piano.up(Data1);
        }
        return;
    }

    char desiredKey = 'x';
    char keyLocation = 'm';

    if (NOTE_ON || NOTE_OFF) {

        if (Data1 > 0 && Data1 < 36) {
            desiredKey = lowNotes[abs((int)Data1 - 35)];
            keyLocation = 'l';
            if (!eightyeightkey) {
                logger.AddLog("Low note %c skipped\n", desiredKey);
                return;
            }
        }
        // Okay
        else if (Data1 >= 36 && Data1 <= 96) {
            desiredKey = letterNoteMap[(int)Data1 - 36];
            keyLocation = 'm';
        }
        // High
        else if (Data1 > 96 && Data1 < 122) {
            desiredKey = highNotes[abs((int)Data1 - 97)];
            keyLocation = 'h';
            if (!eightyeightkey) {
                logger.AddLog("High note %c skipped\n", desiredKey);
                return;
            }
        }
        else {
            logger.AddLog("Could not find key %d\n", Data1);
        }
    }

    if CONTROL_CHANGE{ //      http://midi.teragonaudio.com/tech/midispec/ctllist.htm  -   Control Change
        logger.AddLog("Control change: [1]: %04X [2]: %04X\n", Data1, Data2);
        if (Data1 == 0x40) {                //      http://midi.teragonaudio.com/tech/midispec/hold.htm     -   Sustain Pedal
            if (!sustain) {
                logger.AddLog("Skipping sustain control\n");
                return;
            }
            if (Data2 >= sustainCutoff && !sustainOn) {
                dyn_sendKeyDown(' ');
                sustainOn = true;
                logger.AddLog("Sustain down");
            }
            else if (Data2 < sustainCutoff && sustainOn) {
                dyn_sendKeyUp(' ', 'm');
                sustainOn = false;
                logger.AddLog("Sustain up");
            }
            return;
        }
    }

    if NOTE_ON{  // NoteOn
        piano.down(Data1, Data2);

        if (Data2 == 0) {
            dyn_sendKeyUp(desiredKey, keyLocation);
            return;
        }

        if (velocity == true) {
            static char prevVelocity = 'X'; // init to somebs
            char velocity = findVelocity(Data2);
            if (prevVelocity == velocity) {
                logger.AddLog("Same velocity, skipping ");
            }
            logger.AddLog("Velocity: %c\n", velocity);
            dyn_setVelocity(velocity);
            prevVelocity = velocity;
        }
        else {
            logger.AddLog("Skipping velocity: off\n");
        }

        if (keyLocation == 'm')
        {
            dyn_sendKeyUp(desiredKey, 'm'); // last ditch effort?
            dyn_sendKeyDown(desiredKey);
        }
        else {
            dyn_sendOutOfRangeKey(desiredKey);
        }

        logger.AddLog("Note %c, location: %c\n", desiredKey, keyLocation);
        return;

    }
    if NOTE_OFF{  // NoteOff
        piano.up(Data1);

        logger.AddLog("Releasing %c\n", desiredKey);
        dyn_sendKeyUp(desiredKey, keyLocation);
        return;
    }
    logger.AddLog("%s: status: %x, %d, %d\n", timestampString(timestamp).c_str(), status, Data1, Data2);
}