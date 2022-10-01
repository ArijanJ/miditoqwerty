#include "settings.h"

std::string Setting::GetName() {
	return name;
}

Setting::Setting(std::string name, int* valueRef) {
	this->name = name;
	this->valueRef = valueRef;
}

void* Setting::GetValue() {
	//printf("Value of %s is %d\n", this->GetName().c_str(), *valueRef);
	return valueRef;
}

void Setting::SetValue(int value) {
	int* val = valueRef;
	*val = value;
	//printf("new value of whatever: %d\n", *val);
}

void SettingsHandler::AddSetting(std::string name, int* valueRef) {
	Setting set(name, valueRef);
	settings.push_back(set);
}

bool SettingsHandler::LoadSettings() {
	printf("Attempting to load settings.dat\n");
	int nLine = 1;
	FILE* pSettingsFile ;
	if (fopen_s(&pSettingsFile, "settings.dat", "r") == ERROR_SUCCESS && pSettingsFile != NULL) {
		std::string settingName;
		char buf[225] = { 'a' };
		while (fgets(buf, sizeof(buf), pSettingsFile)) // Next line
		{
			if (buf[strlen(buf) - 1] == '\n')
				buf[strlen(buf) - 1] = '\0'; // get rid of newline
			if (buf[0] == '!') { // custom stuff (theme & no font cause ??)
				if (strstr(buf, "!theme")) {
					printf("Found !theme\n");
					char theme[128];
					sscanf_s(buf, "%*s %s", theme);
					printf("Loading default theme %s\n", theme);
					LoadTheme(theme);
					continue;
				}	
			}
			if (nLine % 2 == 1){
				printf("Reading name: %s\n", buf);
				settingName = buf;
				printf("SETTING IS \"%s\"\n", settingName.c_str());
			}
			else if (nLine % 2 == 0) { // every 2 lines set the setting to the setting's set setting
				printf("Reading value: %s\n", buf);
				printf("Setting %s to %d\n\n", settingName.c_str(), std::stoi(buf));
				this->GetSetting(settingName)->SetValue(std::stoi(buf));
			}
			nLine++; // fun fact i forgot this critical line
		}
		fclose(pSettingsFile);
		printf("Loading settings successful\n");
		return true;
	}
	printf("Error occured while reading settings\n");
	return false;
}

Setting* SettingsHandler::GetSetting(std::string match) {
	for(Setting& setting : settings) {
		std::string name = setting.GetName();
		int* val;
		val = static_cast<int*>(setting.GetValue());
		if (setting.GetName() == match) {
			
			printf("Found setting: %s, value: %d\n", match.c_str(), *val);
			return &setting;
		}
	}
	return nullptr;
}

int* SettingsHandler::GetValue(std::string name) {
	for (Setting& setting : settings) {
		std::string name = setting.GetName();
		int* val;
		val = static_cast<int*>(setting.GetValue());
		printf("Setting name: %s, value: %d\n", name.c_str(), *val);
		if (setting.GetName() == name) {
			return val;
		}
	}
	return NULL;
}

bool SettingsHandler::DumpSettings() {
	printf("Attempting to dump settings\n");
	FILE* pSettingsFile;
	if (fopen_s(&pSettingsFile, "settings.dat", "w") == ERROR_SUCCESS && pSettingsFile != NULL) {
		for (Setting& setting : settings) {
			printf("Dumping %s\n", setting.GetName().c_str());;
			fprintf(pSettingsFile, "%s\n", setting.GetName().c_str());
			fprintf(pSettingsFile, "%d\n", *((int*)setting.GetValue()));
		}

		const char* themeString = currentTheme.c_str();
		// Save theme
		printf("Saving theme: %s\n", themeString);
		fprintf(pSettingsFile, "!theme %s\n", themeString);

		fclose(pSettingsFile);
		printf("Dumping settings successful\n");
		return true;
	}
	printf("Error occured while dumping settings\n");
	return false;
}