#pragma once
#include "generator.h"
#include "utils.h"
#include "logging.h"
#include "modules/lfsrRand.h"

namespace Cyber
{
    class Noise : public SlotGenerator
    {
        Modules::Lfsr rand;
        int divider = 1;
        int phasor = 0;
        float output = 0;
    
    public:
        inline Noise()
        {
            strcpy(TabName, "NOISE");
            ParamCount = 1;
            Param[0] = 0.0;
            ParamUpdated();
        }

        inline float ScaleParameter(int idx, float value)
        {
            if (idx == 0) return (int)(1 + Utils::Resp2dec(value) * 127);
            return 0;
        }

        virtual inline void ParamUpdated() override 
        { 
            divider = ScaleParameter(0, Param[0]);
        }

        virtual inline const char* GetParamName(int idx) override
        {
                 if (idx == 0) return "Reduce";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, float value, char* dest) override
        {
            if (idx == 0) sprintf(dest, "1/%d", (int)ScaleParameter(idx, value));
            else strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args) override
        {
            phasor++;
            rand.Update();
            
            if (phasor > divider)
            {
                phasor -= divider;
                output = rand.GetFloat();
            }
            args->Output = output;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Noise";
            info.GeneratorId = "S-GNA-Noise";
            info.Info = "Basic noise generator";
            info.Version = 1000;
            return info;
        }
    };
}
