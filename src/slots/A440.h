#pragma once
#include "generator.h"
#include "utils.h"
#include "logging.h"
#include "modules/wavetable.h"

namespace Cyber
{
    class A440Hz : public SlotGenerator
    {
        int pitch;
        float gain;
        uint32_t inc;
        uint32_t phasor;
    
    public:
        inline A440Hz()
        {
            strcpy(TabName, "A440");
            ParamCount = 2;
            Param[0] = 0.5;
            Param[1] = 0.5;
            ParamUpdated();
        }

        virtual inline void ParamUpdated(int idx = -1) override 
        { 
            pitch = int(400 + Param[0] * 80.99);
            gain = Param[1] > 0 ? Utils::DB2Gainf(-30 + Param[1] * 30) : 0;
            inc = Modules::Wavetable::GetPhaseIncrement(pitch);
        }

        virtual inline const char* GetParamName(int idx)
        {
                 if (idx == 0) return "Pitch";
            else if (idx == 1) return "Volume";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, char* dest)
        {
            if (idx == 0) 
                sprintf(dest, "%d", pitch);
            else if (idx == 1) 
            {
                if (Param[idx] > 0)
                    sprintf(dest, "%.1f", (-30 + Param[idx] * 30));
                else
                    strcpy(dest, "Off");
            }
            else
                strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args)
        {
            phasor += inc;
            args->Output = Modules::Wavetable::Sin(phasor) * gain;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "A440Hz";
            info.GeneratorId = "Slot-GNA-A440Hz";
            info.Info = "Tuning oscillator.";
            info.Version = 1000;
            return info;
        }
    };
}
