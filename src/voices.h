#pragma once
#include <stdint.h>
#include "generator.h"

namespace Cyber
{
    class Voice
    {
    public:
        float InGain = 1;
        float OutGain = 1;
        uint8_t MidiChannel = 0;
        uint8_t AudioInLeft = 0;
        uint8_t AudioInRight = 0;
        uint8_t AudioOutLeft = 0;
        uint8_t AudioOutRight = 0;
        uint8_t CvIn = 0;
        uint8_t GateIn = 0;
        bool StereoInput = false;
        bool StereoOutput = false;

        Generator* generator;
        
    };

    extern Voice Voices[4];
    extern uint8_t ActiveVoice;
}
