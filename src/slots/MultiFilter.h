#pragma once
#include "generator.h"
#include "utils.h"
#include "logging.h"
#include "modules/biquad.h"
#include "modules/filterCascade.h"

namespace Cyber
{
    class MultiFilter : public SlotGenerator
    {
        const int CUTOFF = 0;
        const int RESONANCE = 1;
        const int DRIVE = 2;
        const int MODE = 3;
    
        float gainOut = 1.0;
        int mode = 0;
        Modules::Biquad biq;
        Modules::FilterCascade cascade;
        
    public:
        inline MultiFilter()
        {
            strcpy(TabName, "MFILT");
            ParamCount = 4;
            Param[CUTOFF] = 0.9;
            Param[RESONANCE] = 0.2;
            Param[DRIVE] = 0.4;
            Param[MODE] = 0.0;

            biq.Type = Modules::Biquad::FilterType::LowPass;
            biq.SetGainDb(0);
            biq.SetSamplerate(SAMPLERATE);
            biq.SetQ(1);
            biq.Frequency = 1500;
            biq.Update();

            cascade.CutoffHz = 5000;
            cascade.Resonance = 0.5;
            cascade.Drive = 0.5;
            
            ParamUpdated();
        }

        inline float ScaleParameter(int idx, float value)
        {
            if (idx == CUTOFF) return value;
            if (idx == RESONANCE) return value;
            if (idx == DRIVE) return value;
            if (idx == MODE) return (int)(value * 6.999);
            return 0;
        }

        virtual inline void ParamUpdated() override 
        { 
            mode = ScaleParameter(MODE, Param[MODE]);
            float cutoff = ScaleParameter(CUTOFF, Param[CUTOFF]);
            float res = ScaleParameter(RESONANCE, Param[RESONANCE]);
            float drive = ScaleParameter(DRIVE, Param[DRIVE]);

            if (mode == 0)
            {
                //nothing
            }
            else if (mode <= 3)
            {
                cascade.CutoffHz = 20 + Utils::Resp4dec(cutoff) * 19980;
                cascade.Resonance = res;
                cascade.Drive = drive;
                if (mode == 1) cascade.SetMode(Modules::InternalFilterMode::Lp24);
                if (mode == 2) cascade.SetMode(Modules::InternalFilterMode::Lp12);
                if (mode == 3) cascade.SetMode(Modules::InternalFilterMode::Bp12_12);
                gainOut = 1.0f;
                cascade.Update();
            }
            else
            {
                cutoff = 20 + Utils::Resp4dec(cutoff) * 19980;
                res = 0.5 + res * 20;
                drive = 0.1 + drive * 0.9;
                biq.Frequency = cutoff;
                biq.SetQ(res);
                if (mode == 4) biq.Type = Modules::Biquad::FilterType::LowPass;
                if (mode == 5) biq.Type = Modules::Biquad::FilterType::BandPass;
                if (mode == 6) biq.Type = Modules::Biquad::FilterType::HighPass;
                biq.Update();
                gainOut = drive;
            }

            // compensate for low volume of bandpass filters at low resonance settings
            if (mode == 3 || mode == 5)
                gainOut = gainOut * (4.0f - res);
        }

        virtual inline const char* GetParamName(int idx) override
        {
                 if (idx == CUTOFF) return "Cutoff";
            else if (idx == RESONANCE) return "Resonance";
            else if (idx == DRIVE) return "Drive";
            else if (idx == MODE) return "Mode";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, float value, char* dest) override
        {
            if (idx == CUTOFF)
                sprintf(dest, "%.0f%%", value*100);
            else if (idx == RESONANCE)
                sprintf(dest, "%.0f%%", value*100);
            else if (idx == DRIVE)
                sprintf(dest, "%.0f%%", value*100);
            else if (idx == MODE)
            {
                auto val = (int)ScaleParameter(MODE, Param[MODE]);
                if (val == 0) strcpy(dest, "Bypassed");
                else if (val == 1) strcpy(dest, "Lp Ladder 4P");
                else if (val == 2) strcpy(dest, "Lp Ladder 2P");
                else if (val == 3) strcpy(dest, "Bp Ladder 2P");
                else if (val == 4) strcpy(dest, "Lp Biquad 12db");
                else if (val == 5) strcpy(dest, "Bp Biquad 12db");
                else if (val == 6) strcpy(dest, "Hp Biquad 12db");
            }
            else strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args) override
        {
            if (mode == 0)
            {
                args->Output = args->Input;
            }
            else if (mode <= 3)
            {
                args->Output = cascade.ProcessSample(args->Input) * gainOut;
            }
            else
            {
                args->Output = biq.Process(args->Input);
                args->Output = Utils::tanhm(args->Output * gainOut, 1);
            }
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Multimode Filter";
            info.GeneratorId = "S-GNA-MFilter";
            info.Info = "Multi-mode filter.";
            info.Version = 1000;
            return info;
        }
    };
}
