#pragma once
#include "stdint.h"
#include "io_buffer.h"
#include "modulators.h"
#include "mod_source_dest.h"
#include "menu.h"

namespace Cyber
{
    class Voice;

    struct ModRouting
    {
        ModSource Source;
        ModDest Dest;
        uint8_t Slot;
        float Amount;
        int16_t AmountRaw;
    };

    const int ModRouteCount = 16;

    class ModMatrix
    {
        Menu menu;
        uint8_t activeRoute = 0;

        ModRouting Routes[ModRouteCount];
        float OutputBuffer[BUFFER_SIZE];
        float OutputValue;
    public:
        void InitMenu();
        inline ModMatrix(Voice* parentVoice) 
        {
            this->voice = parentVoice;
            InitMenu(); 
        }

    private:
        inline float* GetSourceBuffer(ModSource source)
        {
            switch (source)
            {
                case ModSource::Off: return fpData->Mod[0]; // hack
                case ModSource::Mod1: return fpData->Mod[0];
                case ModSource::Mod2: return fpData->Mod[1];
                case ModSource::Mod3: return fpData->Mod[2];
                case ModSource::Mod4: return fpData->Mod[3];
                case ModSource::Cv1: return fpData->Cv[0];
                case ModSource::Cv2: return fpData->Cv[1];
                case ModSource::Cv3: return fpData->Cv[2];
                case ModSource::Cv4: return fpData->Cv[3];
                case ModSource::Gate1: return fpData->GateFloat[0];
                case ModSource::Gate2: return fpData->GateFloat[1];
                case ModSource::Gate3: return fpData->GateFloat[2];
                case ModSource::Gate4: return fpData->GateFloat[3];
                case ModSource::Env1: return modulators->OutEnv1;
                case ModSource::Env2: return modulators->OutEnv2;
                case ModSource::Lfo1: return modulators->OutLfo1;
                case ModSource::Lfo2: return modulators->OutLfo2;
            }
            return nullptr;
        }

        inline void AddRouteSlow(int idx)
        {
            auto buf = GetSourceBuffer(Routes[idx].Source);
            OutputValue += buf[0] * Routes[idx].Amount;
        }

        inline void AddRouteFast(int idx)
        {
            auto buf = GetSourceBuffer(Routes[idx].Source);
            Utils::Mix(OutputBuffer, buf, Routes[idx].Amount, BUFFER_SIZE);
        }

    public:
        FpBuffer* fpData;
        Modulators* modulators;
        Voice* voice;

        inline Menu* GetMenu() { return &menu; }

        inline float GetModulationSlow(ModDest dest, uint8_t slot)
        {
            OutputValue = 0.0f;
            for (int i = 0; i < ModRouteCount; i++)
            {
                if (Routes[i].Dest == dest && Routes[i].Slot == slot)
                    AddRouteSlow(i);
            }
            
            return OutputValue;
        }

        inline float* GetModulationFast(ModDest dest, uint8_t slot)
        {
            Utils::ZeroBuffer(OutputBuffer, BUFFER_SIZE);
            for (int i = 0; i < ModRouteCount; i++)
            {
                if (Routes[i].Dest == dest && Routes[i].Slot == slot)
                    AddRouteFast(i);
            }
            
            return OutputBuffer;
        }

        inline const char* GetModLabel(int slot)
        {
            if (slot == 0) return "Amount 1";
            if (slot == 1) return "Amount 2";
            if (slot == 2) return "Amount 3";
            if (slot == 3) return "Amount 4";
            if (slot == 4) return "Amount 5";
            if (slot == 5) return "Amount 6";
            if (slot == 6) return "Amount 7";
            if (slot == 7) return "Amount 8";
            if (slot == 8) return "Amount 9";
            if (slot == 9) return "Amount 10";
            if (slot == 10) return "Amount 11";
            if (slot == 11) return "Amount 12";
            if (slot == 12) return "Amount 13";
            if (slot == 13) return "Amount 14";
            if (slot == 14) return "Amount 15";
            if (slot == 15) return "Amount 16";
            return "-";
        }
    };
}
