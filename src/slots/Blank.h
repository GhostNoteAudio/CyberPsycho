#pragma once
#include "generator.h"

namespace Cyber
{
    class Blank : public SlotGenerator
    {    
    public:
        inline Blank()
        {
            strcpy(TabName, "-");
            ParamCount = 0;
        }

        virtual inline void ParamUpdated(int idx = -1) override 
        { 
        }

        virtual inline const char* GetParamName(int idx)
        {
            return "";
        }

        virtual inline void GetParamDisplay(int idx, char* dest)
        {
            strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args)
        {
            args->Output = args->Input;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Blank";
            info.GeneratorId = "Slot-GNA-Blank";
            info.Info = "Blank Slot";
            info.Version = 1000;
            return info;
        }
    };
}
