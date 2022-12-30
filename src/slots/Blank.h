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

        virtual inline void ParamUpdated() override 
        { 
        }

        virtual inline const char* GetParamName(int idx) override
        {
            return "";
        }

        virtual inline void GetParamDisplay(int idx, float value, char* dest) override
        {
            strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args) override
        {
            args->Output = args->Input;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Blank";
            info.GeneratorId = "S-GNA-Blank";
            info.Info = "Blank Slot";
            info.Version = 1000;
            return info;
        }
    };
}
