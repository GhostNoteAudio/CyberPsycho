#pragma once
#include "generator.h"
#include "utils.h"
#include "logging.h"
#include "modules/biquad.h"

namespace Cyber
{
    class EqShelf : public SlotGenerator
    {
        const int GAINLOW = 0;
        const int FCLOW = 1;
        const int FCHIGH = 2;
        const int GAINHIGH = 3;

        Modules::Biquad biquadLow;
        Modules::Biquad biquadHigh;
    
    public:
        inline EqShelf()
        {
            strcpy(TabName, "SHELF");
            ParamCount = 4;
            Param[0] = 0.5f;
            Param[1] = 0.5f;
            Param[2] = 0.5f;
            Param[3] = 0.5f;

            biquadLow.SetSamplerate(SAMPLERATE);
            biquadLow.Type = Modules::Biquad::FilterType::LowShelf;
            biquadLow.SetGainDb(0);
            biquadLow.SetQ(0.707);
            biquadLow.Frequency = 200;
            biquadLow.Update();

            biquadHigh.SetSamplerate(SAMPLERATE);
            biquadHigh.Type = Modules::Biquad::FilterType::HighShelf;
            biquadHigh.SetGainDb(0);
            biquadHigh.SetQ(0.707);
            biquadHigh.Frequency = 2000;
            biquadHigh.Update();
            
            ParamUpdated();
        }

        inline float ScaleParameter(int idx, float value)
        {
            if (idx == GAINLOW) return -12 + value * 24;
            if (idx == GAINHIGH) return -12 + value * 24;
            if (idx == FCLOW) return 20 +  Utils::Resp3dec(value) * 1980;
            if (idx == FCHIGH) return 1000 + Utils::Resp3dec(value) * 17000;
            return 0;
        }

        virtual inline void ParamUpdated() override 
        { 
            float gLow = ScaleParameter(GAINLOW, Param[GAINLOW]);
            float gHigh = ScaleParameter(GAINHIGH, Param[GAINHIGH]);
            float fLow = ScaleParameter(FCLOW, Param[FCLOW]);
            float fHigh = ScaleParameter(FCHIGH, Param[FCHIGH]);

            biquadLow.Frequency = fLow;
            biquadLow.SetGainDb(gLow);
            biquadLow.Update();

            biquadHigh.Frequency = fHigh;
            biquadHigh.SetGainDb(gHigh);
            biquadHigh.Update();
        }

        virtual inline const char* GetParamName(int idx) override
        {
                 if (idx == GAINLOW) return "Low Gain";
            else if (idx == FCLOW) return "Low Freq";
            else if (idx == FCHIGH) return "High Freq";
            else if (idx == GAINHIGH) return "High Gain";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, float value, char* dest) override
        {
            if (idx == GAINLOW || idx == GAINHIGH) 
            {
                float val = ScaleParameter(idx, Param[idx]);
                sprintf(dest, "%.1fdB", val);
            }
            else if (idx == FCLOW || idx == FCHIGH)
            {
                float val = ScaleParameter(idx, Param[idx]);
                sprintf(dest, "%.0fHz", val);
            }
            else strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args) override
        {
            float f = args->Input;
            f = biquadLow.Process(f);
            f = biquadHigh.Process(f);
            args->Output = f;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Shelf EQ";
            info.GeneratorId = "Slot-GNA-EQShelf";
            info.Info = "High/Low shelving Equaliser";
            info.Version = 1000;
            return info;
        }
    };
}
