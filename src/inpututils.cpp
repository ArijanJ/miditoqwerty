#pragma once
#include "inpututils.h"

unsigned char scanSets[2][26];

std::unordered_map<unsigned char, short> scanSet1Map;
std::unordered_map<unsigned char, short> scanSet2Map;

const int SHIFT_SCAN = getScanCode(VK_SHIFT);
const int SPACE_SCAN = getScanCode(VK_SPACE);
const int CTRL_SCAN = getScanCode(VK_CONTROL);
const int ALT_SCAN = getScanCode(VK_LMENU);
 
// just collapse this in the ide
void loadScansets() {

	scanSet1Map['1'] = 2;
	scanSet1Map['2'] = 3;
	scanSet1Map['3'] = 4;
	scanSet1Map['4'] = 5;
	scanSet1Map['5'] = 6;
	scanSet1Map['6'] = 7;
	scanSet1Map['7'] = 8;
	scanSet1Map['8'] = 9;
	scanSet1Map['9'] = 10;
	scanSet1Map['0'] = 11;
	scanSet1Map[' '] = SPACE_SCAN;

	scanSet1Map['a'] = 3;
	scanSet1Map['a'] = 30;
	scanSet1Map['b'] = 48;
	scanSet1Map['c'] = 46;
	scanSet1Map['d'] = 32;
	scanSet1Map['e'] = 18;
	scanSet1Map['f'] = 33;
	scanSet1Map['g'] = 34;
	scanSet1Map['h'] = 35;
	scanSet1Map['i'] = 23;
	scanSet1Map['j'] = 36;
	scanSet1Map['k'] = 37;
	scanSet1Map['l'] = 38;
	scanSet1Map['m'] = 50;
	scanSet1Map['n'] = 49;
	scanSet1Map['o'] = 24;
	scanSet1Map['p'] = 25;
	scanSet1Map['q'] = 16;
	scanSet1Map['r'] = 19;
	scanSet1Map['s'] = 31;
	scanSet1Map['t'] = 20;
	scanSet1Map['u'] = 22;
	scanSet1Map['v'] = 47;
	scanSet1Map['w'] = 17;
	scanSet1Map['x'] = 45;
	scanSet1Map['y'] = 21;
	scanSet1Map['z'] = 44;
	/*
	* set 1   set 2, 3   usb
		02     16   16   1E  !1       
		03     1E   1E   1F  @ 2      
		04     26   26   20  # 3      
		05     25   25   21  $ 4      
		06     2E   2E   22 % 5       
		07     36   36   23 ^ 6       
		08     3D   3D   24 & 7       
		09     3E   3E   25 * 8       
		0A     46   46   26  (9      
			0B     45   45   27) 0*/

	scanSet2Map['1'] = 0x16;
	scanSet2Map['2'] = 0x1e;
	scanSet2Map['3'] = 0x26;
	scanSet2Map['4'] = 0x25;
	scanSet2Map['5'] = 0x2e;
	scanSet2Map['6'] = 0x36;
	scanSet2Map['7'] = 0x3d;
	scanSet2Map['8'] = 0x3e;
	scanSet2Map['9'] = 0x46;
	scanSet2Map['0'] = 0x45;
	scanSet2Map[' '] = SPACE_SCAN;

	scanSet2Map['a'] = 0x1c;
	scanSet2Map['b'] = 0x32;
	scanSet2Map['c'] = 0x21;
	scanSet2Map['d'] = 0x23;
	scanSet2Map['e'] = 0x24;
	scanSet2Map['f'] = 0x2b;
	scanSet2Map['g'] = 0x34;
	scanSet2Map['h'] = 0x33;
	scanSet2Map['i'] = 0x43;
	scanSet2Map['j'] = 0x3b;
	scanSet2Map['k'] = 0x42;
	scanSet2Map['l'] = 0x4b;
	scanSet2Map['m'] = 0x3a;
	scanSet2Map['n'] = 0x31;
	scanSet2Map['o'] = 0x44;
	scanSet2Map['p'] = 0x4d;
	scanSet2Map['q'] = 0x15;
	scanSet2Map['r'] = 0x2d;
	scanSet2Map['s'] = 0x1b;
	scanSet2Map['t'] = 0x2c;
	scanSet2Map['u'] = 0x3c;
	scanSet2Map['v'] = 0x2a;
	scanSet2Map['w'] = 0x1d;
	scanSet2Map['x'] = 0x22;
	scanSet2Map['y'] = 0x35;
	scanSet2Map['z'] = 0x1a;
}

HKL getLayout() {
	std::cout << GetKeyboardLayout(0) << std::endl;
	return GetKeyboardLayout(0);
}

HKL layout = getLayout();

std::string alphabet1 = "!@#$%^&*()QWERTYUIOPASDFGHJKLZXCVBNM";
std::string alphabet2 = "1234567890qwertyuiopasdfghjklzxcvbnm";

char findIndex(char c) {
	for (int i = 0; i < alphabet1.size(); i++)
		if (alphabet1[i] == c)
			return alphabet2[i];
	return c;
}

void setLayout(HKL l) {
	// Todo? Invasive?
}

unsigned int getScanCode(int code) {
	return MapVirtualKeyExA(code, MAPVK_VK_TO_VSC, layout);
}

unsigned int getScanCodeChar(char c) {
	return MapVirtualKeyA(VkKeyScanExA(c, layout), MAPVK_VK_TO_VSC);
}

void printInput(INPUT input) {
	char name[256];
	int result = GetKeyNameTextA(input.ki.wScan << 16, name, 32);
	BYTE highByte = HIBYTE(input.ki.wScan);
	std::cout << "Input key: " << name << " \t\t| HIBYTE: " << highByte << std::endl;
}

// Works the same regardless of qwerty emulator
void setupInput(INPUT input[], int size, int code1, int code2, bool advanced = false) {
	ZeroMemory(input, sizeof(input));
	for (int i = 0; i < size; i++) {
		INPUT* ip = &input[i];
		ip->type = INPUT_KEYBOARD;
		ip->ki.time = 0;
		ip->ki.dwExtraInfo = 0;

		if (advanced) {
			// Send only the key
			if (code1 == NULL) {
				ip->ki.dwFlags = (i % 2 == 0 ? KEYEVENTF_SCANCODE : KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);
				ip->ki.wScan = (code2);
			}
			else {
				ip->ki.dwFlags = (i < 2 ? KEYEVENTF_SCANCODE : KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);
				ip->ki.wScan = (i % 2 == 0 ? code1 : code2);
			}
		}
		//printInput(*ip);
	}
}
//
void sendKeyDown(char c) {
	SHORT scanResult = VkKeyScanExA(c, layout);
	int keyCode = LOBYTE(scanResult);
	int shiftState = HIBYTE(scanResult);

	// Should not shift
	if (shiftState == 0) {
		INPUT input[1];
		setupInput(input, 1, NULL, NULL);
		input[0].ki.dwFlags = KEYEVENTF_SCANCODE;
		input[0].ki.wScan = getScanCodeChar(findIndex(c));
		SendInput(1, input, sizeof(INPUT));
	}
	// Should shift
	else {
		INPUT input[3];
		setupInput(input, 3, SHIFT_SCAN, getScanCodeChar(findIndex(c)), true);
		SendInput(3, input, sizeof(INPUT));
	}
	/*
	char name[16];
	GetKeyNameTextA((MapVirtualKeyExA(keyCode, MAPVK_VK_TO_VSC, layout) << 16), name, 16);
	*/

	/*std::cout << "keyCode: " << keyCode << std::endl;
	std::cout << "Key: " << name << std::endl;
	std::cout << "shiftState: " << shiftState << std::endl;*/
}

void sendKeyUp(char c, char location) {
	INPUT input[1];
	setupInput(input, 1, NULL, NULL);
	input[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	if (location == 'm')
		input[0].ki.wScan = getScanCodeChar(findIndex(c));
	else
		input[0].ki.wScan = getScanCodeChar(c);
	SendInput(1, input, sizeof(INPUT));
}

void sendOutOfRangeKey(char c) {
	INPUT input[3];
	setupInput(input, 3, CTRL_SCAN, getScanCodeChar(findIndex(c)), true);
	SendInput(3, input, sizeof(INPUT));
	//std::cout << "Sending out of range key " << c << std::endl;
}

void setVelocity(char c) {
	INPUT input[4];
	setupInput(input, 4, ALT_SCAN, getScanCodeChar(c), true);
	SendInput(4, input, sizeof(INPUT));
}


/////////////////////////////////////////////////
//                                             //
//          QWERTY-EMULATING FUNCTIONS         //
//                                             //
/////////////////////////////////////////////////


// Returns the scan code in the chosen scanset
unsigned int qwerty_getScanCodeChar(char c) {
	return (scanSetChoice ? scanSet2Map[c] : scanSet1Map[c]);
}

void qwerty_sendKeyDown(char c) {
	//SHORT scanResult = qwerty_getScanCodeChar(c);
	//int keyCode = LOBYTE(scanResult);
	//int shiftState = HIBYTE(scanResult);

	// Should not shift
	char findIndexResult = findIndex(c);
	if(findIndexResult == c){
		INPUT input[1];
		setupInput(input, 1, NULL, NULL);
		input[0].ki.dwFlags = KEYEVENTF_SCANCODE;
		input[0].ki.wScan = qwerty_getScanCodeChar(findIndexResult);
		SendInput(1, input, sizeof(INPUT));
	}
	// Should shift
	else {
		INPUT input[3];
		setupInput(input, 3, SHIFT_SCAN, qwerty_getScanCodeChar(findIndexResult), true);
		SendInput(3, input, sizeof(INPUT));
	}
}

void qwerty_sendKeyUp(char c, char location) {
	INPUT input[1];
	setupInput(input, 1, NULL, NULL);
	input[0].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	if (location == 'm')
		input[0].ki.wScan = qwerty_getScanCodeChar(findIndex(c));
	else
		input[0].ki.wScan = qwerty_getScanCodeChar(c);
	SendInput(1, input, sizeof(INPUT));
}

void qwerty_sendOutOfRangeKey(char c) {
	INPUT input[3];
	setupInput(input, 3, CTRL_SCAN, qwerty_getScanCodeChar(findIndex(c)), true);
	SendInput(3, input, sizeof(INPUT));
	//std::cout << "Sending out of range key " << c << std::endl;
}

void qwerty_setVelocity(char c) {
	INPUT input[4];
	setupInput(input, 4, ALT_SCAN, qwerty_getScanCodeChar(c), true);
		//setupInput(input, 4, ALT_SCAN, getScanCodeChar(c), true);
		//SendInput(4, input, sizeof(INPUT));
	SendInput(4, input, sizeof(INPUT));
}