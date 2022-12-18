#pragma once
#include "generator.h"
#include "arm_math.h"
#include "utils.h"
#include "logging.h"
#include "modules/percussionEnvelope.h"

namespace Cyber
{
    class Kick1 : public SlotGenerator
    {
        enum Params
        {
            DECAY = 0,
            PDEC = 1,
            PMOD = 2,
            FREQ = 3,
            BOOST = 4,
            FOLD = 5,
            PSHAPE = 6,
            ASHAPE = 7,
        };

        const float fsinv = 1.0 / SAMPLERATE;

        float phasor = 0;
        bool currentGate = false;
        float adecay = 0;
        float pdecay = 0;
        float pmod = 0;
        float freq = 0;
        float boost = 0;
        float fold = 0;
        float pshape = 0;
        float ashape = 0;

        Modules::PercussionEnvelope ampEnv;
        Modules::PercussionEnvelope pitchEnv;

    public:
        inline Kick1()
        {
            strcpy(TabName, "KICK1");
            ParamCount = 8;
            Param[0] = 0.62f;
            Param[1] = 0.5f;
            Param[2] = 0.7f;
            Param[3] = 0.2f;
            Param[4] = 0.2f;
            Param[5] = 0.0f;
            Param[6] = 0.7f;
            Param[7] = 0.5f;
            ParamUpdated();

            ampEnv.DecaySamples = 20000;
            ampEnv.AttackRate = 1;
            ampEnv.ValueFloorDb = -60;
            ampEnv.UpdateParams();

            pitchEnv.DecaySamples = 20000;
            pitchEnv.AttackRate = 1;
            pitchEnv.ValueFloorDb = -60;
            pitchEnv.UpdateParams();
        }

        virtual inline void ParamUpdated(int idx = -1) override 
        { 
            // Generator only updates when a new sound is triggered.
            // no live updates
        }

        virtual inline const char* GetParamName(int idx)
        {
                 if (idx == 0) return "Decay";
            else if (idx == 1) return "Pitch Dec";
            else if (idx == 2) return "Pitch Mod";
            else if (idx == 3) return "Freq";
            else if (idx == 4) return "Boost";
            else if (idx == 5) return "Fold";
            else if (idx == 6) return "Pitch Crv";
            else if (idx == 7) return "Amp Crv";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, char* dest)
        {
            if (idx == FREQ || idx == PMOD)
            {
                float freq = GetScaledParameter(idx);
                sprintf(dest, "%.0f Hz", freq);
            }
            else
            {
                sprintf(dest, "%.1f", Param[idx] * 100);
            }
        }

        virtual inline void Process(SlotArgs* args)
        {
            auto g = args->Gate;
            if (!currentGate && g)
            {
                phasor = 0;
                UpdateAll();
            }
            currentGate = g;

            float aenv = ampEnv.Process(g);
            float penv = pitchEnv.Process(g);
            phasor += (pmod * penv + freq) * fsinv;
            float s = arm_sin_f32(phasor * 2 * M_PI) * aenv;

            if (fold > 0)
                s = arm_sin_f32(s * fold);
            if (boost > 1)
                s = tanhf(s * boost);

            args->Output = s;
        }

        inline void UpdateAll()
        {
            adecay = GetScaledParameter(DECAY);
            pdecay = GetScaledParameter(PDEC);
            pmod = GetScaledParameter(PMOD);
            freq = GetScaledParameter(FREQ);
            boost = GetScaledParameter(BOOST);
            fold = GetScaledParameter(FOLD);
            pshape = GetScaledParameter(PSHAPE);
            ashape = GetScaledParameter(ASHAPE);

            ampEnv.DecaySamples = adecay;
            pitchEnv.DecaySamples = pdecay;
            ampEnv.ValueFloorDb = -ashape;
            pitchEnv.ValueFloorDb = -pshape;
            ampEnv.UpdateParams();
            pitchEnv.UpdateParams();
        }

        inline float GetScaledParameter(int idx)
        {
            float val = Param[idx];

            if (idx == DECAY) return (0.01 + Utils::Resp1dec(val) * 0.99) * SAMPLERATE * 2;
            if (idx == PDEC) return (0.002 + Utils::Resp1dec(val) * 0.5) * SAMPLERATE;
            if (idx == PMOD) return Utils::Resp2dec(val) * 1000;
            if (idx == FREQ) return Utils::Resp2dec(val) * 1000;
            if (idx == BOOST) return 1 + val * 20;
            if (idx == FOLD) return val * 10;
            if (idx == PSHAPE) return 10 + val * 50;
            if (idx == ASHAPE) return 10 + val * 50;
            return 0;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Kick One";
            info.GeneratorId = "Slot-GNA-Kick1";
            info.Info = "X0X Style Kickdrum";
            info.Version = 1000;
            return info;
        }
    };
}
