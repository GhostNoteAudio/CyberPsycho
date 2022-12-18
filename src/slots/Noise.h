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

        virtual inline void ParamUpdated(int idx = -1) override 
        { 
            divider = 1 + Utils::Resp2dec(Param[0]) * 127;
        }

        virtual inline const char* GetParamName(int idx)
        {
                 if (idx == 0) return "Redux";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, char* dest)
        {
            if (idx == 0) sprintf(dest, "%d", divider);
            else strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args)
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
            info.GeneratorId = "Slot-GNA-Noise";
            info.Info = "Basic noise generator";
            info.Version = 1000;
            return info;
        }
    };
}
