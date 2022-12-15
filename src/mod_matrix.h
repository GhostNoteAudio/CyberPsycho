#pragma once
#include "stdint.h"
#include "io_buffer.h"
#include "mod_source.h"
#include "menu.h"

namespace Cyber
{
    class Voice;

    struct ModRouting
    {
        ModSource Source;
        uint8_t Slot;
        float Amount;
    };

    const int ModRouteCount = 16;

    class ModMatrix
    {
        Menu menu;
        uint8_t activeRoute = 0;

        float OutputBuffer[BUFFER_SIZE];
        float OutputValue;

    public:
        ModRouting Routes[ModRouteCount];
        int LastUpdatedRoute = -1;
        void InitMenu();
        inline ModMatrix() 
        {
            InitMenu(); 
        }

        void UpdateMenuDisplay();
        void UpdateRoute(ModSource source, int slot, float value);

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

        inline Menu* GetMenu() { return &menu; }

        inline float GetModulationSlow(uint8_t slot)
        {
            OutputValue = 0.0f;
            for (int i = 0; i < ModRouteCount; i++)
            {
                if (Routes[i].Slot == slot)
                    AddRouteSlow(i);
            }
            
            return OutputValue;
        }

        inline float* GetModulationFast(uint8_t slot)
        {
            Utils::ZeroBuffer(OutputBuffer, BUFFER_SIZE);
            for (int i = 0; i < ModRouteCount; i++)
            {
                if (Routes[i].Slot == slot)
                    AddRouteFast(i);
            }
            
            return OutputBuffer;
        }
    };
}
