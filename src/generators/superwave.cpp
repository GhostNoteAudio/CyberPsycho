#include "superwave.h"
#include "menu.h"
#include "modules/wavetable.h"
#include "logging.h"

namespace Cyber
{
    const int SEMI = 0;
    const int PSPREAD = 1;
    const int VSPREAD = 2;
    const int CENT = 3;

    const int COUNT = 7;
    const float OFFSETS[7] = {-0.1102, -0.0629, -0.0235, 0.0, 0.0217, 0.0593, 0.1056};
    const float GAINS[7] = {0.1, 0.3, 0.7, 1, 0.65, 0.32, 0.08};
    
    Superwave::Superwave() 
        : biqL(Modules::Biquad::FilterType::HighPass, SAMPLERATE)
        , biqR(Modules::Biquad::FilterType::HighPass, SAMPLERATE)
    {
        menu.Captions[SEMI] = "Semi";
        menu.Captions[PSPREAD] = "Spread";
        menu.Captions[VSPREAD] = "Mix";
        menu.Captions[CENT] = "Cent";

        menu.Min[SEMI] = -36;
        menu.Min[CENT] = -50;
        menu.Steps[SEMI] = 72;
        menu.Steps[CENT] = 100;

        menu.Values[SEMI] = 0.5f;
        menu.Values[PSPREAD] = 0.5f;
        menu.Values[VSPREAD] = 0.5f;
        menu.Values[CENT] = 0.5f;

        menu.SetLength(4);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;

        for (int i = 0; i < COUNT; i++)
        {
            phasorL[i] = ((uint32_t)rand()) * 2;
            phasorR[i] = ((uint32_t)rand()) * 2;
        }
        biqL.SetQ(0.707);
        biqR.SetQ(0.707);
        Update();
    }

    void Superwave::Update(float pitchHz)
    {
        float vspread = GetScaledParameter(VSPREAD);

        for (int i = 0; i < COUNT; i++)
        {
            volumes[i] = GAINS[i] * (1-vspread) + vspread;
            volumes[i] = volumes[i] * volumes[i];
        }
        
        biqL.Frequency = pitchHz;
        biqR.Frequency = pitchHz;
        biqL.Update();   
        biqR.Update();   
        gainAdjust = 1.0 / Utils::Sum(volumes, 7);
    }

    float Superwave::GetScaledParameter(int idx)
    {
        if (idx == SEMI) return menu.GetScaledValue(SEMI);
        if (idx == CENT) return menu.GetScaledValue(CENT);
        if (idx == PSPREAD) return menu.Values[PSPREAD];
        if (idx == VSPREAD) return menu.Values[VSPREAD];
        return 0;
    }

    Menu* Superwave::GetMenu()
    {
        return &menu;
    }

    void Superwave::Process(GeneratorArgs args)
    {
        float pspread = GetScaledParameter(PSPREAD);
        float pitch = args.Cv[0] * 12 + GetScaledParameter(SEMI) + GetScaledParameter(CENT) * 0.01;
        float pitchHz = Utils::Note2HzLut(pitch);
        Update(pitchHz);

        for (int n = 0; n < args.Size; n++)
        {
            float output = 0;
            for (int i=0; i<COUNT; i++)
            {
                float hz = pitchHz * (1 + OFFSETS[i] * pspread);
                uint32_t inc = Modules::Wavetable::GetPhaseIncrement(hz);
                phasorL[i] += inc;
                float v = (phasorL[i] - 0x7FFFFFFF) * 4.6566128e-10f;
                output += v * volumes[i];
            }
            
            output *= gainAdjust;
            output = biqL.Process(output); // high pass to remove aliasing below fundamental
            output += 0.2 * Modules::Wavetable::Sin(phasorL[3]); // increase fundamental frequency to compensate for high pass
            args.OutputLeft[n] = output * 0.8;
        }

        if (!args.Stereo)
            return;

        for (int n = 0; n < args.Size; n++)
        {
            float output = 0;
            for (int i=0; i<COUNT; i++)
            {
                float hz = pitchHz * (1 + OFFSETS[i] * pspread * 1.01);
                uint32_t inc = Modules::Wavetable::GetPhaseIncrement(hz);
                phasorR[i] += inc;
                float v = (phasorR[i] - 0x7FFFFFFF) * 4.6566128e-10f;
                output += v * volumes[i];
            }
            
            output *= gainAdjust;
            output = biqR.Process(output); // high pass to remove aliasing below fundamental
            output += 0.2 * Modules::Wavetable::Sin(phasorR[3]); // increase fundamental frequency to compensate for high pass
            args.OutputRight[n] = output * 0.8;
        }
    }
}
