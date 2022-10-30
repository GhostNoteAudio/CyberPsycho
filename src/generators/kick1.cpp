#include "kick1.h"
#include "menu.h"

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
        menu.Captions[7] = "Aamp Crv";

        menu.Values[0] = 20;
        menu.Values[1] = 20;
        menu.Values[2] = 20;
        menu.Values[3] = 3;
        menu.Values[4] = 2;
        menu.Values[5] = 0;
        menu.Values[6] = 50;
        menu.Values[7] = 50;

        menu.SetLength(8);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;
        
        ampEnv.Mode = Modules::Envelope::EnvMode::AR;
        ampEnv.AttackMode = Modules::Envelope::CurveMode::Linear;
        ampEnv.ReleaseMode = Modules::Envelope::CurveMode::Exp;
        ampEnv.AttackSamples = 0;
        ampEnv.ReleaseSamples = 20000;
        ampEnv.OneShot = true;
        ampEnv.ResetOnTrig = true;
        ampEnv.UpdateParams();

        pitchEnv.Mode = Modules::Envelope::EnvMode::AR;
        pitchEnv.AttackMode = Modules::Envelope::CurveMode::Linear;
        pitchEnv.ReleaseMode = Modules::Envelope::CurveMode::Exp;
        pitchEnv.AttackSamples = 0;
        pitchEnv.ReleaseSamples = 20000;
        pitchEnv.OneShot = true;
        pitchEnv.ResetOnTrig = true;
        pitchEnv.UpdateParams();
    }

    float Kick1::GetScaledParameter(int idx)
    {
        if (idx == DECAY) return (0.01 + menu.Values[DECAY] * 0.01 *0.99) * SAMPLERATE;
        if (idx == PDEC) return (0.002 + menu.Values[PDEC] * 0.01 * 0.5) * SAMPLERATE;
        if (idx == PMOD) return menu.Values[PMOD] * 10;
        if (idx == FREQ) return 10 + menu.Values[FREQ] * 2.90;
        if (idx == BOOST) return 1 + menu.Values[BOOST] * 0.2;
        if (idx == FOLD) return menu.Values[FOLD] * 0.1;
        if (idx == PSHAPE) return 10 + menu.Values[PSHAPE] * 0.5;
        if (idx == ASHAPE) return 10 + menu.Values[ASHAPE] * 0.5;
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
        float pshape = GetScaledParameter(PSHAPE);
        float ashape = GetScaledParameter(ASHAPE);

        ampEnv.ReleaseSamples = adecay;
        pitchEnv.ReleaseSamples = pdecay;
        ampEnv.UpdateParams(-ashape);
        pitchEnv.UpdateParams(-pshape);

        for (int i = 0; i < args.Size; i++)
        {
            auto g = args.Gate[i];
            if (!currentGate && g)
                phasor = 0;
            currentGate = g;

            float aenv = ampEnv.Process(g);
            float penv = pitchEnv.Process(g);
            phasor += (pmod * penv + freq) * fsinv;
            float s = sinf(phasor * 2 * M_PI) * aenv;

            if (fold > 0)
                s = sinf(s * fold);
            if (boost > 1)
                s = tanhf(s * boost);

            args.OutputLeft[i] = s;
        }
    }
}
