#pragma once
#include "generator.h"
#include "utils.h"
#include "logging.h"
#include "modules/blitOsc.h"
#include "modules/slewLimiter.h"
#include "modules/additiveOsc.h"

namespace Cyber
{
    class BasicWaves : public SlotGenerator
    {
        const int SEMI = 0;
        const int WAVE = 1;
        const int ADJUST = 2;
        const int CENT = 3;
        const int SLEW = 4;

        float pitchCv = 0;

        Modules::SlewLimiter slew;
        Modules::BlitOsc saw;
        Modules::BlitOsc pulse;
        Modules::AdditiveOsc triangle;
        Modules::AdditiveOsc sine;

    public:
        inline BasicWaves() : 
            saw(Modules::BlitOsc::Waveform::Saw, 200, true), 
            pulse(Modules::BlitOsc::Waveform::Pulse, 200, true)
        {
            slew.Rate = 10;
            triangle.SetTriangle();
            sine.SetTone(0, 1, 1);

            strcpy(TabName, "BASIC");
            ParamCount = 5;
            Param[SEMI] = 0.5;
            Param[WAVE] = 0.0;
            Param[ADJUST] = 0.0;
            Param[CENT] = 0.5;
            Param[SLEW] = 0.0;
            ParamUpdated();
        }

        inline float ScaleParameter(int idx, float value)
        {
            if (idx == SEMI) return round(-24 + (int)(value * 48.999));
            if (idx == WAVE) return (int)(value * 6.999);
            if (idx == ADJUST) return value;
            if (idx == CENT) return value - 0.5;
            if (idx == SLEW) return 0.001 + Utils::Resp4dec(1-value) * 9.999;
            return 0;
        }

        virtual inline void ParamUpdated() override 
        {
            float semi = ScaleParameter(SEMI, Param[SEMI]);
            float cent = ScaleParameter(CENT, Param[CENT]);
            float pitch = pitchCv * CV_RANGE * 12 + semi + cent;
            slew.Rate = ScaleParameter(SLEW, Param[SLEW]);
            pitch = slew.Update(pitch);
            //LogInfof("Pitch: %.3f", pitch);
            float pitchHz = Utils::Note2HzLut(pitch);
            float period = SAMPLERATE / pitchHz;
            saw.setPeriod(period);
            pulse.setPeriod(period);

            float adj = ScaleParameter(ADJUST, Param[ADJUST]);
            saw.setPwm(adj);
            pulse.setPwm(adj);

            triangle.SetFrequency(pitchHz);
            sine.SetFrequency(pitchHz);
        }

        virtual inline const char* GetParamName(int idx) override
        {
                 if (idx == SEMI) return "Semi";
            else if (idx == WAVE) return "Wave";
            else if (idx == ADJUST) return "Adjust";
            else if (idx == CENT) return "Cent";
            else if (idx == SLEW) return "Slewrate";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, float value, char* dest) override
        {
            if (idx == SEMI) sprintf(dest, "%d", (int)ScaleParameter(idx, value));
            else if (idx == WAVE) 
            {
                auto sv = ScaleParameter(WAVE, value);
                if (sv == 0) strcpy(dest, "Saw");
                if (sv == 1) strcpy(dest, "Pulse");
                if (sv == 2) strcpy(dest, "Triangle");
                if (sv == 3) strcpy(dest, "Sine");
                if (sv == 4) strcpy(dest, "FM1");
                if (sv == 5) strcpy(dest, "FM2");
                if (sv == 6) strcpy(dest, "FM3");
            }
            else if (idx == ADJUST) sprintf(dest, "%d%%", (int)(ScaleParameter(idx, value)*100));
            else if (idx == CENT) sprintf(dest, "%d", (int)(ScaleParameter(idx, value)*100));
            else if (idx == SLEW)
            {
                auto p = ScaleParameter(idx, value);
                if (p < 0.01)
                    sprintf(dest, "%.4f", p);
                else if (p < 0.1)
                    sprintf(dest, "%.3f", p);
                else if (p < 1)
                    sprintf(dest, "%.2f", p);
                else
                    sprintf(dest, "%.1f", p);
            }
            else strcpy(dest, "");
        }

        virtual inline void Process(SlotArgs* args) override
        {
            pitchCv = pitchCv * 0.9 + args->Cv * 0.1;
            auto sv = ScaleParameter(WAVE, Param[WAVE]);
            
            float samplePulse = pulse.tick();
            float sampleSaw = saw.tick();
            float sampleTri = triangle.Process();
            float sampleSine = sine.Process();

            if (sv == 0)
                args->Output = sampleSaw;
            else if (sv == 1)
                args->Output = samplePulse;
            else if (sv == 2)
                args->Output = sampleTri;
            else if (sv == 3)
                args->Output = sampleSine;
            else
                args->Output = 0;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Basic Waveforms";
            info.GeneratorId = "S-GNA-BASIC";
            info.Info = "Basic waveform generator";
            info.Version = 1000;
            return info;
        }
    };
}
