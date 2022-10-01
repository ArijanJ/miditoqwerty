#pragma once
#include <iostream>
#include <Windows.h>
#include <unordered_map>

// http://www.quadibloc.com/comp/scan.htm
extern std::unordered_map<unsigned char, short> scanSet1Map;
extern std::unordered_map<unsigned char, short> scanSet2Map;

extern int scanSetChoice;

void loadScansets();

HKL getLayout();

void setLayout(HKL l);

void setupInput(INPUT input[], int size, int code1, int code2, bool advanced);

unsigned int getScanCode(int code);

unsigned int getScanCodeChar(char c);

void sendKeyDown(char c);

void sendKeyUp(char c, char location = 'm');

void sendOutOfRangeKey(char c);

void setVelocity(char c);


// qwerty-emulating functions

unsigned int qwerty_getScanCodeChar(char c);

void qwerty_sendKeyDown(char c);

void qwerty_sendKeyUp(char c, char location = 'm');

void qwerty_sendOutOfRangeKey(char c);

void qwerty_setVelocity(char c);

//void type(std::string string);