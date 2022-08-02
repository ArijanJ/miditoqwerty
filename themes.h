#pragma once

#include <stdio.h>
#include <math.h>
#include <filesystem>

extern ImVec4 gBackgroundColor;
extern ImVec4 gNoteColor;

extern std::string currentFont;
extern std::string currentTheme;

extern std::string defaultFont;
extern std::string defaultTheme;

int LoadTheme(std::string path);
bool ShowThemeSelector(const char* label, std::string& output);
void ImGui::ShowFontSelector(const char* label);
bool ImGui::ShowStyleSelector(const char* label);
void ImGui::ShowStyleEditor(ImGuiStyle* ref);