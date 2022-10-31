#pragma once
#include <stdint.h>
#include "generator.h"
#include "io_buffer.h"
#include "generators/multimodeFilter.h"

namespace Cyber
{
    class Voice
    {
    public:
        MultimodeFilter mmf;

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

        Generator* Gen = nullptr;
        int GenIndex = 0;

        void Process(FpBuffer* fpData);
        
    };

    namespace Voices
    {
        extern Voice Voices[4];
        extern uint8_t ActiveVoice;

        inline Voice* GetActiveVoice()
        {
            return &Voices[ActiveVoice];
        }

        inline Generator* GetActiveGen()
        {
            return Voices[ActiveVoice].Gen;
        }
    }
}
