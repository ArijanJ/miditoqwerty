//
// Created by Chris Young on 25/4/20.
//

#include <vector>
#include <iostream>
#include "util.h"
#include "imgui.h"

void advanceRainbow(int* isRed, int* isGreen, int* isBlue) {

    const int unit = 5;

    //if (ImGui::GetFrameCount() % 1 != 0) return; // this is what vsync is for dumbass
    if (*isGreen == 1 && *isBlue == 0)
    {
        *isRed += unit;
    }

    if (*isRed > 252 && *isBlue == 0)
    {
        *isRed = 255;
        *isGreen += unit;
    }

    if (*isGreen > 252 && *isBlue == 0)
    {
        *isGreen = 255;
        *isRed -= unit;
    }

    if (*isRed < 1 && *isGreen == 255)
    {
        *isRed = 0;
        *isBlue += unit;
    }

    if (*isBlue > 252 && *isRed == 0)
    {
        *isBlue = 255;
        *isGreen -= unit;
    }

    if (*isGreen < 1 && *isBlue == 255)
    {
        *isGreen = 0;
        *isRed += unit;
    }

    if (*isRed > 252 && *isGreen == 0)
    {
        *isRed = 255;
        *isBlue -= unit;
    }

    if (*isBlue < 1 && *isGreen == 0)
    {
        *isBlue = 0;
        //*isRed -= unit;
        if (*isRed < 1)
            *isGreen = 1;   
    }
    /*float colors[3] = { *isRed / 255, *isGreen / 255, *isBlue / 255 };
    ImGui::ColorPicker3("AAAA", colors); debug shii*/
}

std::string midiNoteString(uint8_t note) {
    static const char *base[] = {
            "C", "C#",
            "D", "D#",
            "E",
            "F", "F#",
            "G", "G#",
            "A", "A#",
            "B"
    };
    int octave = note / 12 - 1;
    note %= 12;
    char buf[50];
    snprintf(buf, sizeof buf, "%s%d", base[note], octave);
    return std::string(buf);
}

std::string midiChordString(std::vector<int> notes) {
    char buf[50];
    if (notes.size() == 3) {
        if (notes[2]-notes[1] == 3 && notes[1]-notes[0] == 4) {
            snprintf(buf, sizeof buf, "%s major", midiNoteString(notes[0]).c_str());
            return std::string(buf);
        } else if (notes[2]-notes[1] == 4 && notes[1]-notes[0] == 3) {
            snprintf(buf, sizeof buf, "%s minor", midiNoteString(notes[0]).c_str());
            return std::string(buf);
        }
    }
    return std::string("");
}

std::string timestampString(PmTimestamp timestamp) {
    char buf[100] = {0};

    unsigned int millis = timestamp % 1000;
    timestamp /= 1000;

    unsigned int seconds = timestamp % 60;
    timestamp /= 60;

    unsigned int minutes = timestamp % 60;
    timestamp /= 60;

    unsigned int hours = timestamp % 24;
    timestamp /= 24;

    snprintf(buf, sizeof buf, "%02d:%02d:%02d.%03d", hours, minutes, seconds, millis);
    return std::string(buf);
}
