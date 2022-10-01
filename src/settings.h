#pragma once
#include <vector>
#include <string>
#include <fmt/format.h>
#include <winerror.h>
#include <stdio.h>
#include <imgui.h>
#include "themes.h"

class Setting {
	std::string name;
	int*		valueRef;
public:
	Setting(std::string name, int* valueRef);
	std::string GetName();
	void* GetValue();
	void SetValue(int value);
};

class SettingsHandler {
	std::vector<Setting> settings; // lol
public:
	bool LoadSettings();
	void AddSetting(std::string name, int* valueRef);

	Setting* GetSetting(std::string name);
	int* GetValue(std::string name);
	bool DumpSettings();
};