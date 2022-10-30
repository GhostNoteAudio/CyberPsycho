#include "superwave.h"
#include "menu.h"
#include "modules/wavetable.h"

namespace Cyber
{
    const int SEMI = 0;
    const int PSPREAD = 1;
    const int VSPREAD = 2;
    const int CENT = 3;

    const int COUNT = 7;
    const float OFFSETS[7] = {-0.1102, -0.0629, -0.0235, 0.0, 0.0217, 0.0593, 0.1056};
    const float GAINS[7] = {0.1, 0.3, 0.7, 1, 0.65, 0.32, 0.08};
    
    Superwave::Superwave() : biq(Modules::Biquad::FilterType::HighPass, SAMPLERATE)
    {
        menu.Captions[SEMI] = "Semi";
        menu.Captions[PSPREAD] = "Pitch Sprd";
        menu.Captions[VSPREAD] = "Vol Sprd";
        menu.Captions[CENT] = "Cent";

        menu.Min[SEMI] = -36;
        menu.Max[SEMI] = 36;
        menu.Min[CENT] = -50;
        menu.Max[CENT] = 50;

        menu.Values[SEMI] = 0;
        menu.Values[PSPREAD] = 50;
        menu.Values[VSPREAD] = 50;
        menu.Values[CENT] = 0;

        menu.SetLength(4);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;

        for (int i = 0; i < COUNT; i++)
        {
            phasor[i] = ((uint32_t)rand()) * 2;
        }
        biq.SetQ(0.707);
        Reset();
    }

    void Superwave::Reset(float pitchHz)
    {
        float vspread = GetScaledParameter(VSPREAD);
        float pitch = 60 + GetScaledParameter(SEMI) + GetScaledParameter(CENT) * 0.01;

        for (int i = 0; i < COUNT; i++)
        {
            volumes[i] = GAINS[i] * (1-vspread) + vspread;
            volumes[i] = volumes[i] * volumes[i];
        }
        
        biq.Frequency = pitchHz < 0 ? Utils::Note2hz(pitch) : pitchHz;
        biq.Update();   
        gainAdjust = 1.0 / Utils::Sum(volumes, 7);
    }

    float Superwave::GetScaledParameter(int idx)
    {
        if (idx == SEMI) return menu.Values[SEMI];
        if (idx == CENT) return menu.Values[CENT];
        if (idx == PSPREAD) return menu.Values[PSPREAD] * 0.01;
        if (idx == VSPREAD) return menu.Values[VSPREAD] * 0.01;
        return 0;
    }


    Menu* Superwave::GetMenu()
    {
        return &menu;
    }

    void Superwave::Process(GeneratorArgs args)
    {
        float pspread = GetScaledParameter(PSPREAD);
        float pitch = 60 + GetScaledParameter(SEMI) + GetScaledParameter(CENT) * 0.01;
        float pitchHz = Utils::Note2hz(pitch);
        Reset(pitchHz);
        //LogInfof("pspread: %.3f, pitch: %.3f", pspread, pitch);

        for (int n = 0; n < args.Size; n++)
        {
            float output = 0;
            for (int i=0; i<COUNT; i++)
            {
                float hz = pitchHz * (1 + OFFSETS[i] * pspread);
                uint32_t inc = Modules::Wavetable::GetPhaseIncrement(hz);
                phasor[i] += inc;
                float v = (phasor[i] - 0x7FFFFFFF) * 4.6566128e-10f;
                output += v * volumes[i];
            }
            
            output *= gainAdjust;
            output = biq.Process(output); // high pass to remove aliasing below fundamental
            output += 0.2 * Modules::Wavetable::SinFast(phasor[3]); // increase fundamental frequency to compensate for high pass

            args.OutputLeft[n] = output * 0.8;
        }
    }
}
