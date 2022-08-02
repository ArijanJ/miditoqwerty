#include "imgui.h"
#include "themes.h"

#define IM_NEWLINE  "\r\n"

#define IM_MIN(A, B)            (((A) < (B)) ? (A) : (B))
#define IM_MAX(A, B)            (((A) >= (B)) ? (A) : (B))

namespace ImGui { IMGUI_API void ShowFontAtlas(ImFontAtlas* atlas); }

void ImGui::ShowFontSelector(const char* label)
{
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font_current = ImGui::GetFont();
    if (ImGui::BeginCombo(label, font_current->GetDebugName()))
    {
        for (int n = 0; n < io.Fonts->Fonts.Size; n++)
        {
            ImFont* font = io.Fonts->Fonts[n];
            ImGui::PushID((void*)font);
            if (ImGui::Selectable(font->GetDebugName(), font == font_current))
                io.FontDefault = font;
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
}

bool ImGui::ShowStyleSelector(const char* label)
{
    static int style_idx = -1;
    if (ImGui::Combo(label, &style_idx, "Dark\0Light\0Classic\0"))
    {
        switch (style_idx)
        {
        case 0: ImGui::StyleColorsDark(); break;
        case 1: ImGui::StyleColorsLight(); break;
        case 2: ImGui::StyleColorsClassic(); break;
        }
        return true;
    }

    return false;
}

bool ShowThemeSelector(const char* label, std::string& output) {

    static bool gotThemes = false;
    static std::vector<std::string> themes;

    if (!gotThemes) {
        for (auto& p : std::filesystem::recursive_directory_iterator("themes"))
        {
            if (p.path().extension() == ".theme") {
                printf("Got theme %s\n", p.path().stem().string().c_str());
                themes.push_back(p.path().stem().string());
            }
        }
        gotThemes = true;
    }

    if (themes.size() == 0) return "NO_THEMES_FOUND";

    static std::string current_item = currentTheme;

    bool retval = false;

    static bool value_changed = false;

    ImGui::Text("Theme");
    if (ImGui::BeginCombo("##combo", current_item.c_str())) // The second parameter is the label previewed before opening the combo.
    {
        for (int n = 0; n < themes.size(); n++)
        {
            bool is_selected = (current_item == themes.at(n)); // You can store your selection however you want, outside or inside your objects
            if (ImGui::Selectable(themes.at(n).c_str(), is_selected))
                current_item = themes.at(n);
                output = current_item;
                LoadTheme("themes/" + current_item + ".theme");

                value_changed = true;
                retval = true;
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
        }
        ImGui::EndCombo();
    }

    return retval;
}

int LoadTheme(std::string path) {
    static std::string previousPath = ""; // this is sooo bad lol
    if (previousPath == path) return -1;

    // save currentTheme
    currentTheme = path;

    previousPath = path;

    FILE* fp;
    if (fopen_s(&fp, path.c_str(), "r") != 0) return -1;
    if (!fp) return -2;
    // while no eof

    ImGuiStyle& style = ImGui::GetStyle();         

    char buf[255];
    int settingNo = 0;
    while (fgets(buf, sizeof(buf), fp)) { // Next line
        //sscanf_s(buf, "")
        float red; float green; float blue; float alpha;
        // fprintf_s(fp, "ImGuiCol_%s %f %f %f %f\n", name, col.x, col.y, col.z, col.w); <-- INPUT
        sscanf_s(buf, "%f %f %f %f %*s\n", &red, &green, &blue, &alpha);
        printf("Picked up ImGuiCol_%-32.32s %f %f %f %f\n", ImGui::GetStyleColorName(settingNo), red, green, blue, alpha);
        style.Colors[settingNo] = { red, green, blue, alpha };
        settingNo++;
    }

    gBackgroundColor = style.Colors[ImGuiCol_WindowBg];
    printf("gBackgroundColor set to %.2f, %.2f, %.2f, %.2f\n", gBackgroundColor.x, gBackgroundColor.y, gBackgroundColor.z, gBackgroundColor.w);
    gNoteColor       = style.Colors[ImGuiCol_SliderGrab];
    printf("gNoteColor set to %.2f, %.2f, %.2f, %.2f\n", gNoteColor.x, gNoteColor.y, gNoteColor.z, gNoteColor.w);

    gNoteNameColor   = style.Colors[ImGuiCol_Text];
    printf("midiNoteNamesColor set to %.2f, %.2f, %.2f, %.2f\n", gNoteNameColor.x, gNoteNameColor.y, gNoteNameColor.z, gNoteNameColor.w);

    fclose(fp);

    return 0;
}

void ImGui::ShowStyleEditor(ImGuiStyle* ref)
{
    // You can pass in a reference ImGuiStyle structure to compare to, revert to and save to
    // (without a reference style pointer, we will use one compared locally as a reference)
    ImGuiStyle& style = ImGui::GetStyle();
    //pStyle = &style;
    static ImGuiStyle ref_saved_style;

    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.75f);
    
    ImGui::ShowFontSelector("Fonts##Selector");

    //printf("Selected theme %d\n", currentTheme);
    std::string curTheme; // not to be confused with currentTheme
    ShowThemeSelector("Theme", curTheme);
    ImGui::SameLine();//ImGui::Separator();

            if (ImGui::Button("Save"))
            {
                /*if (output_dest == 0)
                    ImGui::LogToClipboard();
                else*/
                ImGui::LogToTTY();
                ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
                FILE* fp;
                //std::string destination = "themes/" + currentTheme + ".theme";
                //currentTheme = destination;
                if (fopen_s(&fp, currentTheme.c_str(), "w") == 0) {
                    for (int i = 0; i < ImGuiCol_COUNT; i++)
                    {
                        const ImVec4& col = style.Colors[i];
                        const char* name = ImGui::GetStyleColorName(i);

                        fprintf_s(fp, "%f %f %f %f ImGuiCol_%s\n", col.x, col.y, col.z, col.w, name);
                    }
                    fclose(fp);
                }
                ImGui::LogFinish();
                printf("Saved theme %s\n", currentTheme.c_str());
            }

            static ImGuiTextFilter filter;
            filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

            /*static ImGuiColorEditFlags alpha_flags = 0;
            if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None)) { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
            if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
            if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();*/
            /*HelpMarker(
                "In the color list:\n"
                "Left-click on color square to open color picker,\n"
                "Right-click to open edit options menu.");*/

            ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
            ImGui::PushItemWidth(-160);
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                const char* name = ImGui::GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                ImGui::PushID(i);
                ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_NoInputs);
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
                {
                    // Tips: in a real user application, you may want to merge and use an icon font into the main font,
                    // so instead of "Save"/"Revert" you'd use icons!
                    // Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
                }
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                ImGui::TextUnformatted(name);
                ImGui::PopID();
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();

    ImGui::PopItemWidth();
}