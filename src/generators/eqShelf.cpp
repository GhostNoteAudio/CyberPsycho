#include "eqShelf.h"
#include "menu.h"

namespace Cyber
{
    const int GAINLOW = 0;
    const int FCLOW = 1;
    const int FCHIGH = 2;
    const int GAINHIGH = 3;

    EQShelf::EQShelf()
    {
        menu.Captions[GAINLOW] = "Low Gain";
        menu.Captions[FCLOW] = "Low Freq";
        menu.Captions[FCHIGH] = "High Freq";
        menu.Captions[GAINHIGH] = "High Gain";

        menu.Max[GAINLOW] = 1023;
        menu.Max[FCLOW] = 1023;
        menu.Max[FCHIGH] = 1023;
        menu.Max[GAINHIGH] = 1023;

        menu.Values[GAINLOW] = 512;
        menu.Values[FCLOW] = 512;
        menu.Values[FCHIGH] = 512;
        menu.Values[GAINHIGH] = 512;
        
        menu.Formatters[GAINLOW] = [this](int idx, int16_t value, char* target)
        {
            float val = GetScaledParameter(idx);
            sprintf(target, "%.1fdB", val);
        };

        menu.Formatters[FCLOW] = [this](int idx, int16_t value, char* target)
        {
            float val = GetScaledParameter(idx);
            sprintf(target, "%.0fHz", val);
        };

        menu.Formatters[GAINHIGH] = menu.Formatters[GAINLOW];
        menu.Formatters[FCHIGH] = menu.Formatters[FCLOW];

        menu.SetLength(4);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;

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
    }

    float EQShelf::GetScaledParameter(int idx)
    {
        if (idx == GAINLOW) return -12 + menu.Values[GAINLOW] * 9.77517e-4 * 24;
        if (idx == GAINHIGH) return -12 + menu.Values[GAINHIGH] * 9.77517e-4 * 24;
        if (idx == FCLOW) return 20 +  Utils::Resp3dec(menu.Values[FCLOW] * 9.77517e-4) * 1980;
        if (idx == FCHIGH) return 1000 + Utils::Resp3dec(menu.Values[FCHIGH] * 9.77517e-4) * 17000;
        return 0;
    }

    Menu* EQShelf::GetMenu()
    {
        return &menu;
    }

    void EQShelf::Process(GeneratorArgs args)
    {
        float gLow = GetScaledParameter(GAINLOW);
        float gHigh = GetScaledParameter(GAINHIGH);
        float fLow = GetScaledParameter(FCLOW);
        float fHigh = GetScaledParameter(FCHIGH);

        biquadLow.Frequency = fLow;
        biquadLow.SetGainDb(gLow);
        biquadLow.Update();

        biquadHigh.Frequency = fHigh;
        biquadHigh.SetGainDb(gHigh);
        biquadHigh.Update();

        biquadLow.Process(args.InputLeft, args.OutputLeft, args.Size);
        biquadHigh.Process(args.OutputLeft, args.OutputLeft, args.Size);
    }
}
