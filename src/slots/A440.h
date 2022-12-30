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

        inline float ScaleParameter(int idx, float value)
        {
            if (idx == 0) return int(400 + value * 80.99);
            if (idx == 1) return value > 0 ? Utils::DB2Gainf(-30 + value * 30) : 0;
            return 0;
        }

        virtual inline void ParamUpdated() override 
        { 
            pitch = ScaleParameter(0, Param[0]);
            gain = ScaleParameter(1, Param[1]);
            inc = Modules::Wavetable::GetPhaseIncrement(pitch);
        }

        virtual inline const char* GetParamName(int idx) override
        {
                 if (idx == 0) return "Pitch";
            else if (idx == 1) return "Volume";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, float value, char* dest) override
        {
            if (idx == 0) 
                sprintf(dest, "%d", (int)ScaleParameter(0, value));
            else if (idx == 1) 
            {
                if (value > 0)
                    sprintf(dest, "%.1f", ScaleParameter(1, value));
                else
                    strcpy(dest, "Off");
            }
            else
                strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args) override
        {
            phasor += inc;
            args->Output = Modules::Wavetable::Sin(phasor) * gain;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "A440Hz";
            info.GeneratorId = "S-GNA-A440Hz";
            info.Info = "Tuning oscillator.";
            info.Version = 1000;
            return info;
        }
    };
}
