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
        menu.Captions[1] = "Pitch Dec";
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

        menu.Formatters[2] = [this](int idx, float value, int sv, char* target)
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

    float Kick1::GetScaledParameter(int idx, float modulation)
    {
        float val = Utils::Clamp(menu.Values[idx] + modulation);

        if (idx == DECAY) return (0.01 + val * 0.99) * SAMPLERATE * 2;
        if (idx == PDEC) return (0.002 + val * 0.5) * SAMPLERATE;
        if (idx == PMOD) return val * 1000;
        if (idx == FREQ) return 10 + val * 290;
        if (idx == BOOST) return 1 + val * 20;
        if (idx == FOLD) return val * 10;
        if (idx == PSHAPE) return 10 + val * 50;
        if (idx == ASHAPE) return 10 + val * 50;
        return 0;
    }

    Menu* Kick1::GetMenu()
    {
        return &menu;
    }

    void Kick1::SetTab(int tab)
    {
        if (tab == 0)
            menu.SetSelectedItem(0);
        if (tab == 1)
            menu.SetSelectedItem(4);

        ActiveTab = tab;
    }

    int Kick1::GetModSlots()
    {
        return 8;
    }
    
    void Kick1::GetModSlotName(int idx, char* dest)
    {
        strcpy(dest, menu.Captions[idx]);
    }

    int Kick1::ResolveSlot(int knobIdx)
    {
        return menu.TopItem + knobIdx;
    }

    void Kick1::UpdateAll(GeneratorArgs args)
    {
        // Updates are triggered at trigger events. Realtime updates not allowed
        float fmod = powf(2, args.GetModulationSlow(3));

        adecay = GetScaledParameter(DECAY, args.GetModulationSlow(0));
        pdecay = GetScaledParameter(PDEC, args.GetModulationSlow(1));
        pmod = GetScaledParameter(PMOD, args.GetModulationSlow(2));
        freq = GetScaledParameter(FREQ) * fmod;
        boost = GetScaledParameter(BOOST, args.GetModulationSlow(4));
        fold = GetScaledParameter(FOLD, args.GetModulationSlow(5));
        pshape = GetScaledParameter(PSHAPE, args.GetModulationSlow(6));
        ashape = GetScaledParameter(ASHAPE, args.GetModulationSlow(7));

        ampEnv.DecaySamples = adecay;
        pitchEnv.DecaySamples = pdecay;
        ampEnv.ValueFloorDb = -ashape;
        pitchEnv.ValueFloorDb = -pshape;
        ampEnv.UpdateParams();
        pitchEnv.UpdateParams();
    }

    void Kick1::Process(GeneratorArgs args)
    {
        float fsinv = 1.0 / SAMPLERATE;
        
        if (adecay == 0) // check for initial start
            UpdateAll(args);

        for (int i = 0; i < args.Size; i++)
        {
            auto g = args.Data->Gate[0][i];
            if (!currentGate && g)
            {
                phasor = 0;
                UpdateAll(args);
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

            args.Data->Out[0][i] = s;
        }
    }
}
