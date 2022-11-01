#include "redux.h"
#include "menu.h"

namespace Cyber
{
    const int RATE = 0;
    const int BITCRUSH = 1;
    const int GAIN = 2;
    
    Redux::Redux()
    {
        menu.Captions[RATE] = "Rate Reduce";
        menu.Captions[BITCRUSH] = "Bitcrush";
        menu.Captions[GAIN] = "Gain";
        
        menu.Min[RATE] = 1;
        menu.Max[RATE] = 1000;

        menu.Min[BITCRUSH] = 100;
        menu.Max[BITCRUSH] = 1200;

        menu.Values[RATE] = 1;
        menu.Values[BITCRUSH] = 1200;
        menu.Values[GAIN] = 50;

        for (int i = 0; i < 3; i++)
        {
            menu.Formatters[i] = [this](int idx, int16_t value, char* target)
            {
                float val = GetScaledParameter(idx);
                sprintf(target, "%.1f", val);
            };
        }
        
        menu.SetLength(3);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;
    }

    float Redux::GetScaledParameter(int idx)
    {
        if (idx == RATE) return 1 + Utils::Resp3dec(menu.Values[RATE] * 0.001) * 63;
        if (idx == BITCRUSH) return menu.Values[BITCRUSH] * 0.01;
        if (idx == GAIN) return -12 + 24 * menu.Values[GAIN] * 0.01;
        return 0;
    }


    Menu* Redux::GetMenu()
    {
        return &menu;
    }

    void Redux::Process(GeneratorArgs args)
    {
        float rate = GetScaledParameter(RATE);
        float bits = GetScaledParameter(BITCRUSH);
        float gain = Utils::DB2Gainf(GetScaledParameter(GAIN));

        float levels = powf(2, bits);
        float levelsInv = 1.0f/levels;

        for (int i = 0; i < args.Size; i++)
        {
            float s = args.InputLeft[i];

            // rate reduce
            if (rate > 1)
            {
                phasor += 1.0f;
                if (phasor > rate)
                {
                    phasor -= rate;
                    sample = args.InputLeft[i];
                }
                s = sample;
            }

            s *= gain;

            // bitcrush
            if (rate < 12)
                s = ((int)(s * levels))*levelsInv;

            args.OutputLeft[i] = s;
        }
    }
}
