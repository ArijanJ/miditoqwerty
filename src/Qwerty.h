#pragma once
#include <string>

std::string lowNotes = "trewq0987654321";
std::string letterNoteMap = "1!2@34$5%6^78*9(0qQwWeErtTyYuiIoOpPasSdDfgGhHjJklLzZxcCvVbBnm";
std::string highNotes = "yuiopasdfghj";
//std::string fullScale = "trewq09876543211!2@34$5%6^78*9(0qQwWeErtTyYuiIoOpPasSdDfgGhHjJklLzZxcCvVbBnmyuiopasdfghj";

std::string velocities = "1234567890qwertyuiopasdfghjklzxc";

int velocityList[] = {
4,   8,   12,  16,
20,  24,  28,  32,
36,  40,  44,  48,
52,  56,  60,  64,
68,  72,  76,  80,
84,  88,  92,  96,
100, 104, 108, 112,
116, 120, 124, 127 };

constexpr char findVelocity(int required) {
	if (required <= 4)
		return '1';
	for (int index = 0; index < (sizeof(velocityList) / sizeof(int)) - 1; index++) {
		int curr = velocityList[index];
		int next = velocityList[index + 1];
		if ((curr <= required && next >= required) || next == 127)
		{
			return velocities[index + 1];
		}
	}
	return 't';
}