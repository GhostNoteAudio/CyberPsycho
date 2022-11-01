#include "basicDrive.h"
#include "menu.h"

namespace Cyber
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
    
    BasicDrive::BasicDrive()
    {
        menu.Captions[LOWCUT] = "Low Cut";
        menu.Captions[DRIVE] = "Drive";
        menu.Captions[MODE] = "Mode";
        menu.Captions[ROLLOFF] = "Rolloff";

        menu.Max[MODE] = 4;
        
        menu.Formatters[LOWCUT] = [this](int idx, int16_t value, char* target)
        {
            float val = GetScaledParameter(idx);
            const char* str = idx == LOWCUT ? "Hz" : "";
            sprintf(target, "%.1f%s", val, str);
        };

        menu.Formatters[DRIVE] = [this](int idx, int16_t value, char* target)
        {
            sprintf(target, "%d%%", value);
        };

        menu.Formatters[MODE] = [this](int idx, int16_t value, char* target)
        {
            if (value == MODE_BYPASS) strcpy(target, "Bypass");
            else if (value == MODE_TANH) strcpy(target, "Tanh");
            else if (value == MODE_ASSYM) strcpy(target, "Assym");
            else if (value == MODE_CLIP) strcpy(target, "Clip");
            else if (value == MODE_FOLD) strcpy(target, "Fold");
        };

        menu.Formatters[ROLLOFF] = [this](int idx, int16_t value, char* target)
        {
            sprintf(target, "%d%%", value);
        };
        
        menu.SetLength(4);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;

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
    }

    float BasicDrive::GetScaledParameter(int idx)
    {
        if (idx == LOWCUT) return 10 + Utils::Resp3dec(menu.Values[LOWCUT] * 0.01) * 1990;
        if (idx == DRIVE) return Utils::Resp3dec(menu.Values[DRIVE] * 0.01);
        if (idx == MODE) return menu.Values[MODE];
        if (idx == ROLLOFF) return menu.Values[ROLLOFF] * 0.01;
        return 0;
    }

    Menu* BasicDrive::GetMenu()
    {
        return &menu;
    }

    void BasicDrive::Process(GeneratorArgs args)
    {
        float lowcutFreq = GetScaledParameter(LOWCUT);
        float drive = 1 + GetScaledParameter(DRIVE) * 20;
        int mode = menu.Values[MODE];
        float rolloff = 100 + GetScaledParameter(ROLLOFF)*19900;

        biquadHp.Frequency = lowcutFreq;
        biquadHp.Update();

        biquadLp.Frequency = rolloff;
        biquadLp.Update();

        if (mode == 0)
        {
            Utils::Copy(args.OutputLeft, args.InputLeft, args.Size);
            return;
        }

        for (int i = 0; i < args.Size; i++)
        {
            float s = args.InputLeft[i];
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
            args.OutputLeft[i] = s;
        }
    }
}
