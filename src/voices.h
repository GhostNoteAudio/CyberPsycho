#pragma once
#include <stdint.h>
#include "generator.h"
#include "io_buffer.h"
#include "generators/bypass.h"
#include "generatorRegistry.h"
#include "modulators.h"
#include "menu.h"
#include "mod_matrix.h"

namespace Cyber
{
    class Voice
    {
        Menu menu;
        
    public:
        float InGain = 1;
        float OutGain = 1;
        int8_t PitchOffset = 0;
        int8_t MidiChannel = 0;
        int8_t AudioInLeft = 0;
        int8_t AudioInRight = 1;
        int8_t AudioOutLeft = 0;
        int8_t AudioOutRight = 1;
        int8_t CvIn = 0;
        int8_t GateIn = 0;
        int8_t AmpControl = 0;
        bool StereoProcessing = true;
        
        Generator* Gen = nullptr;
        Generator* Inserts[4] = {nullptr};
        Modulators modulators;
        ModMatrix matrix;

        int ActiveInsert = 0;

        inline Voice() : matrix(this)
        {

        }

        void InitMenu();

        inline void Init()
        {
            for (int i = 0; i < 4; i++)
                Inserts[i] = generatorRegistry.CreateInstanceById("Bypass");

            Gen = generatorRegistry.CreateInstanceById("Bypass");
            InitMenu();
        }

        inline const char* GetModLabel(int slot)
        {
            if (slot == 0) return "Pitch";
            if (slot == 1) return "Gain In";
            if (slot == 2) return "Gain Out";
            return "-";
        }

        inline Menu* GetMenu() { return &menu; }
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
