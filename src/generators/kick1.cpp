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

    Kick1::Kick1()
    {
        menu.Captions[0] = "Decay";
        menu.Captions[1] = "Pitch Dec.";
        menu.Captions[2] = "Pitch Mod";
        menu.Captions[3] = "Freq";
        menu.Captions[4] = "Boost";
        menu.Captions[5] = "Fold";

        menu.Values[0] = 20;
        menu.Values[1] = 20;
        menu.Values[2] = 20;
        menu.Values[3] = 3;
        menu.Values[4] = 2;
        menu.Values[5] = 0;

        menu.SetLength(6);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;
        /*
        ampEnv.Mode = Modules::Envelope::EnvMode::AR;
        ampEnv.AttackMode = Modules::Envelope::CurveMode::Linear;
        ampEnv.ReleaseMode = Modules::Envelope::CurveMode::Exp;
        ampEnv.AttackSamples = 10;
        ampEnv.ReleaseSamples = 44000;
        ampEnv.UpdateParams();
        */
    }

    float Kick1::GetScaledParameter(int idx)
    {
        if (idx == DECAY) return 0.01 + menu.Values[DECAY] * 0.02;
        if (idx == PDEC) return 0.001 + menu.Values[PDEC] * 0.002;
        if (idx == PMOD) return menu.Values[PMOD] * 10;
        if (idx == FREQ) return 10 + menu.Values[FREQ] * 2.90;
        if (idx == BOOST) return 1 + menu.Values[BOOST] * 0.2;
        if (idx == FOLD) return menu.Values[FOLD] * 0.1;
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

        for (int i = 0; i < args.Size; i++)
        {
            auto g = args.Gate[i];
            if (!currentGate && g)
                Trigger();
            currentGate = g;

            float t = index * fsinv;
            float penv = expf(-t/pdecay);
            float aenv = expf(-t/adecay);
            phasor += (pmod * penv + freq) * fsinv;
            float s = sinf(phasor * 2 * M_PI) * aenv;

            //s = redux(s*10, 3)
            if (fold > 0)
                s = Fold(s, fold);
            if (boost > 1)
                s = Boost(s, boost);

            index += 1;
            args.OutputLeft[i] = s;
        }
    }

    void Kick1::ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1)
    {

    }

    void Kick1::ProcessOffline()
    {

    }
}
