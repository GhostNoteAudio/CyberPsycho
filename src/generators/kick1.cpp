#include "kick1.h"
#include "menu.h"
#include "arm_math.h"

namespace Cyber
{
    const int DECAY = 0;
    const int PDEC = 1;
    const int PMOD = 2;
    const int FREQ = 3;
    const int BOOST = 4;
    const int FOLD = 5;
    const int PSHAPE = 6;
    const int ASHAPE = 7;

    Kick1::Kick1()
    {
        menu.Captions[0] = "Decay";
        menu.Captions[1] = "Pitch Dec.";
        menu.Captions[2] = "Pitch Mod";
        menu.Captions[3] = "Freq";
        menu.Captions[4] = "Boost";
        menu.Captions[5] = "Fold";
        menu.Captions[6] = "Pitch Crv";
        menu.Captions[7] = "Amp Crv";

        menu.Values[0] = 0.2f;
        menu.Values[1] = 0.2f;
        menu.Values[2] = 0.2f;
        menu.Values[3] = 0.3f;
        menu.Values[4] = 0.2f;
        menu.Values[5] = 0.0f;
        menu.Values[6] = 0.5f;
        menu.Values[7] = 0.5f;

        menu.Formatters[2] = [this](int idx, float value, char* target)
        {
            float freq = GetScaledParameter(idx);
            sprintf(target, "%.0f Hz", freq);
        };

        menu.Formatters[3] = menu.Formatters[2];

        menu.SetLength(8);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;
        
        ampEnv.DecaySamples = 20000;
        ampEnv.AttackRate = 1;
        ampEnv.ValueFloorDb = -60;
        ampEnv.UpdateParams();

        pitchEnv.DecaySamples = 20000;
        pitchEnv.AttackRate = 1;
        pitchEnv.ValueFloorDb = -60;
        pitchEnv.UpdateParams();
    }

    float Kick1::GetScaledParameter(int idx)
    {
        if (idx == DECAY) return (0.01 + menu.Values[DECAY] * 0.99) * SAMPLERATE * 2;
        if (idx == PDEC) return (0.002 + menu.Values[PDEC] * 0.5) * SAMPLERATE;
        if (idx == PMOD) return menu.Values[PMOD] * 1000;
        if (idx == FREQ) return 10 + menu.Values[FREQ] * 290;
        if (idx == BOOST) return 1 + menu.Values[BOOST] * 20;
        if (idx == FOLD) return menu.Values[FOLD] * 10;
        if (idx == PSHAPE) return 10 + menu.Values[PSHAPE] * 50;
        if (idx == ASHAPE) return 10 + menu.Values[ASHAPE] * 50;
        return 0;
    }


    Menu* Kick1::GetMenu()
    {
        return &menu;
    }

    void Kick1::Process(GeneratorArgs args)
    {
        float fsinv = 1.0 / SAMPLERATE;
        float adecay = GetScaledParameter(DECAY);
        float pdecay = GetScaledParameter(PDEC);
        float pmod = GetScaledParameter(PMOD);
        float freq = GetScaledParameter(FREQ);
        float boost = GetScaledParameter(BOOST);
        float fold = GetScaledParameter(FOLD);
        
        // Todo: These two should change the curve shape of the envelopes
        float pshape = GetScaledParameter(PSHAPE);
        float ashape = GetScaledParameter(ASHAPE);

        ampEnv.DecaySamples = adecay;
        pitchEnv.DecaySamples = pdecay;
        ampEnv.ValueFloorDb = -ashape;
        pitchEnv.ValueFloorDb = -pshape;
        ampEnv.UpdateParams();
        pitchEnv.UpdateParams();

        for (int i = 0; i < args.Size; i++)
        {
            auto g = args.Gate[i];
            if (!currentGate && g)
                phasor = 0;
            currentGate = g;

            float aenv = ampEnv.Process(g);
            float penv = pitchEnv.Process(g);
            phasor += (pmod * penv + freq) * fsinv;
            float s = arm_sin_f32(phasor * 2 * M_PI) * aenv;

            if (fold > 0)
                s = arm_sin_f32(s * fold);
            if (boost > 1)
                s = tanhf(s * boost);

            args.OutputLeft[i] = s;
        }
    }
}
