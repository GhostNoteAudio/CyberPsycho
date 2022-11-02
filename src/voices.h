#pragma once
#include <stdint.h>
#include "generator.h"
#include "io_buffer.h"
#include "generators/bypass.h"
#include "generatorRegistry.h"

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

        Generator* Gen = nullptr;
        Generator* Inserts[4] = {nullptr};

        int ActiveInsert = 0;

        inline void Init()
        {
            for (int i = 0; i < 4; i++)
                Inserts[i] = generatorRegistry.CreateInstanceById("Bypass");

            Gen = generatorRegistry.CreateInstanceById("Bypass");
        }

        inline Generator* GetActiveInsert() { return Inserts[ActiveInsert]; }

        void Process(FpBuffer* fpData);
        
    };

    namespace Voices
    {
        extern Voice Voices[4];
        extern uint8_t ActiveVoice;

        inline void InitVoices()
        {
            Voices[0].Init();
            Voices[1].Init();
            Voices[2].Init();
            Voices[3].Init();
        }

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
