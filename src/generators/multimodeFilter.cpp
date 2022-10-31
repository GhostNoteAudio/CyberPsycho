#include "multimodeFilter.h"
#include "menu.h"

namespace Cyber
{
    const int CUTOFF = 0;
    const int RESONANCE = 1;
    const int DRIVE = 2;
    const int MODE = 3;

    MultimodeFilter::MultimodeFilter()
    {
        menu.Captions[CUTOFF] = "Cutoff";
        menu.Captions[RESONANCE] = "Resonance";
        menu.Captions[DRIVE] = "Drive";
        menu.Captions[MODE] = "Mode";

        menu.Max[CUTOFF] = 1023;
        menu.Max[RESONANCE] = 1023;
        menu.Max[MODE] = 5;

        menu.Values[CUTOFF] = 800;
        menu.Values[RESONANCE] = 100;
        menu.Values[DRIVE] = 0;
        menu.Values[MODE] = 0;

        menu.Formatters[MODE] = [](int idx, int16_t val, char* target)
        {
            if (val == 0) strcpy(target, "LP Ladder 4P");
            else if (val == 1) strcpy(target, "LP Ladder 2P");
            else if (val == 2) strcpy(target, "BP Ladder 2P");
            else if (val == 3) strcpy(target, "LP Biquad 12db");
            else if (val == 4) strcpy(target, "BP Biquad 12db");
            else if (val == 5) strcpy(target, "HP Biquad 12db");
        };

        menu.SetLength(4);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;

        biq.Type = Modules::Biquad::FilterType::LowPass;
        biq.SetGainDb(0);
        biq.SetSamplerate(SAMPLERATE);
        biq.SetQ(1);
        biq.Frequency = 1500;
        biq.Update();

        cascade.Cutoff = 80;
        cascade.Resonance = 0.99;
        cascade.Drive = 0.5;
    }

    Menu* MultimodeFilter::GetMenu()
    {
        return &menu;
    }

    float MultimodeFilter::GetScaledParameter(int idx)
    {
        if (idx == CUTOFF) return menu.Values[CUTOFF] * 9.77517e-4f;
        if (idx == RESONANCE) return menu.Values[RESONANCE] * 9.77517e-4f;
        if (idx == DRIVE) return menu.Values[DRIVE] * 0.01;
        if (idx == MODE) return menu.Values[MODE];
        return 0;
    }

    void MultimodeFilter::Process(GeneratorArgs args)
    {
        int mode = menu.Values[MODE];
        float cutoff = GetScaledParameter(CUTOFF);
        float res = GetScaledParameter(RESONANCE);
        float drive = GetScaledParameter(DRIVE);

        if (mode <= 2)
        {
            cascade.Cutoff = cutoff * 136;
            cascade.Resonance = res;
            cascade.Drive = drive;
            if (mode == 0) cascade.SetMode(Modules::InternalFilterMode::Lp24);
            if (mode == 1) cascade.SetMode(Modules::InternalFilterMode::Lp12);
            if (mode == 2) cascade.SetMode(Modules::InternalFilterMode::Bp12_12);

            cascade.Process(args.InputLeft, args.Size);
            auto dat = cascade.GetOutput();
            for (int i = 0; i < args.Size; i++)
                args.OutputLeft[i] = dat[i] * 0.4;
        }
        else
        {
            cutoff = 20 + Utils::Resp4dec(cutoff) * 19980;
            res = res * 20;
            drive = 0.1 + drive * 0.9;
            biq.Frequency = cutoff;
            biq.SetQ(res);
            if (mode == 3) biq.Type = Modules::Biquad::FilterType::LowPass;
            if (mode == 4) biq.Type = Modules::Biquad::FilterType::BandPass;
            if (mode == 5) biq.Type = Modules::Biquad::FilterType::HighPass;
            biq.Update();

            biq.Process(args.InputLeft, args.OutputLeft, args.Size);
            for (int i = 0; i < args.Size; i++)
                args.OutputLeft[i] = tanhf(args.OutputLeft[i] * drive);
        }
    }

}