#pragma once
#include "generator.h"
#include "utils.h"
#include "logging.h"
#include "modules/biquad.h"

namespace Cyber
{
    class BasicDrive : public SlotGenerator
    {
        const int LOWCUT = 0;
        const int DRIVE = 1;
        const int MODE = 2;
        const int ROLLOFF = 3;

        const int MODE_BYPASS = 0;
        const int MODE_TANH = 1;
        const int MODE_ASSYM = 2;
        const int MODE_CLIP = 3;
        const int MODE_FOLD = 4;
        
        Modules::Biquad biquadHp;
        Modules::Biquad biquadLp;
        float drive;
        int mode;
    
    public:
        inline BasicDrive()
        {
            strcpy(TabName, "DRIVE");
            ParamCount = 4;
            Param[0] = 0.0;
            Param[1] = 0.0;
            Param[2] = 0.0;
            Param[3] = 1.0;

            biquadHp.SetSamplerate(SAMPLERATE);
            biquadHp.Type = Modules::Biquad::FilterType::HighPass;
            biquadHp.SetQ(0.707);
            biquadHp.Frequency = 200;
            biquadHp.Update();

            biquadLp.SetSamplerate(SAMPLERATE);
            biquadLp.Type = Modules::Biquad::FilterType::LowPass6db;
            biquadLp.SetQ(0.707);
            biquadLp.Frequency = 200;
            biquadLp.Update();
            
            ParamUpdated();
        }

        inline float ScaleParameter(int idx, float value)
        {
            if (idx == LOWCUT) return 10 + Utils::Resp3dec(value) * 1990;
            if (idx == DRIVE) return Utils::Resp3dec(value);
            if (idx == MODE) return (int)(value * 3.99);
            if (idx == ROLLOFF) return 100 + Utils::Resp3dec(value) * 19900;
            return 0;
        }

        virtual inline void ParamUpdated() override 
        { 
            drive = 1 + ScaleParameter(DRIVE, Param[DRIVE]) * 20;
            mode = ScaleParameter(MODE, Param[MODE]);
            float lowcutFreq = ScaleParameter(LOWCUT, Param[LOWCUT]);
            float rolloff = ScaleParameter(ROLLOFF, Param[ROLLOFF]);

            biquadHp.Frequency = lowcutFreq;
            biquadHp.Update();

            biquadLp.Frequency = rolloff;
            biquadLp.Update();
        }

        virtual inline const char* GetParamName(int idx) override
        {
                 if (idx == LOWCUT) return "Low Cut";
            else if (idx == DRIVE) return "Drive";
            else if (idx == MODE) return "Mode";
            else if (idx == ROLLOFF) return "Rolloff";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, float value, char* dest) override
        {
            if (idx == LOWCUT) 
                sprintf(dest, "%.1fHz", ScaleParameter(LOWCUT, Param[LOWCUT]));
            else if (idx == MODE)
            {
                auto sv = (int)ScaleParameter(MODE, Param[MODE]);
                if (sv == MODE_BYPASS) strcpy(dest, "Bypass");
                else if (sv == MODE_TANH) strcpy(dest, "Tanh");
                else if (sv == MODE_ASSYM) strcpy(dest, "Assym");
                else if (sv == MODE_CLIP) strcpy(dest, "Clip");
                else if (sv == MODE_FOLD) strcpy(dest, "Fold");
            }
            else strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args) override
        {
            if (mode == 0)
            {
                args->Output = args->Input;
                return;
            }

            float s = args->Input;
            s = biquadHp.Process(s);
            s *= drive;

            if (mode == MODE_TANH)
                s = tanhf(s);
            if (mode == MODE_ASSYM)
                s = tanhf(s + 0.7);
            else if (mode == MODE_CLIP)
                s = Utils::ClipF(s, -1.0f, 1.0f);
            else if (mode == MODE_FOLD)
                s = sinf(s);

            s = biquadLp.Process(s);
            args->Output = s;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Basic Drive";
            info.GeneratorId = "S-GNA-BasicDrive";
            info.Info = "Simple Drive/Distortion effect";
            info.Version = 1000;
            return info;
        }
    };
}
