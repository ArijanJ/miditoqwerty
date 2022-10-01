//
// Created by Chris Young on 22/4/20.
//

#ifndef MIDI_MIDI_H
#define MIDI_MIDI_H

#include <functional>

#include <portmidi.h>
#include "porttime.h"

extern PmTimestamp lastNotePlayed;

class Midi {
public:
    PmDeviceID deviceID;
    PortMidiStream* stream;
    virtual ~Midi();
    void shutdown(PortMidiStream* stream);

private:
    PmEvent buffer[1024];
public:
    void InitWrapper();
    void poll(std::function<void(PmTimestamp, uint8_t, PmMessage, PmMessage)> callback, bool debug = false);
    Midi(PmDeviceID passedID = -1); //constructor

};


#endif //MIDI_MIDI_H
