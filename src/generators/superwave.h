#pragma once

#include "generator.h"
#include "menu.h"
#include "modules/biquad.h"
#include "modules/wavetable.h"
#include "modules/envelope.h"
#include "modules/filterCascade.h"

namespace Cyber
{
    class SuperwaveVoice
    {
        const int COUNT = 7;
        const float OFFSETS[7] = {-0.1102, -0.0629, -0.0235, 0.0, 0.0217, 0.0593, 0.1056};
        const float GAINS[7] = {0.1, 0.3, 0.7, 1, 0.65, 0.32, 0.08};

        uint32_t phasorSub = 0;
        uint32_t phasor[7] = {0};
        float volumes[7] = {0};
        Modules::Biquad biq;
        Modules::Envelope aenv;
        Modules::Envelope fenv;
        Modules::FilterCascade filter;
        float gainAdjust = 1.0;
        float subOscGain = 0.0;
    
    public:
        float PSpread; // 0..1
        float VSpread; // 0..1
        float PitchHz; // 0...20k
        float SubOsc; // 0..1

        float Cutoff;
        float Resonance;
        float Drive;
        float EnvAmt;

        int Aattack; // in samples
        int Adecay; // in samples
        int Asustain;
        int Arelease; // in samples

        int Fattack; // in samples
        int Fdecay; // in samples
        int Fsustain;
        int Frelease; // in samples

        inline SuperwaveVoice() : biq(Modules::Biquad::FilterType::HighPass, SAMPLERATE)
        {
            PSpread = 0.5;
            VSpread = 0.5;
            PitchHz = 100;

            aenv.ReleaseCurve = Modules::Envelope::EnvCurve::Exp;
            fenv.ReleaseCurve = Modules::Envelope::EnvCurve::Exp;
            aenv.Mode = Modules::Envelope::EnvMode::ADSR;
            fenv.Mode = Modules::Envelope::EnvMode::ADSR;

            for (int i = 0; i < COUNT; i++)
            {
                phasor[i] = ((uint32_t)rand()) * 2;
            }
            biq.SetQ(0.707);
            Update();
        }

        inline void Update()
        {
            for (int i = 0; i < COUNT; i++)
            {
                volumes[i] = GAINS[i] * (1-VSpread) + VSpread;
                volumes[i] = volumes[i] * volumes[i];
            }
            
            biq.Frequency = PitchHz;
            biq.Update();
            gainAdjust = 1.0 / Utils::Sum(volumes, 7);
            subOscGain = SubOsc > 0 ? Utils::DB2Gainf(-30 + SubOsc * 30) : 0;

            float cutoff = Cutoff + fenv.GetOutput() * EnvAmt;
            filter.Cutoff = Utils::Clamp(cutoff);
            filter.Resonance = Resonance;
            filter.Drive = Drive;
            filter.Update();

            aenv.AttackSamples = Aattack;
            aenv.DecaySamples = Adecay;
            aenv.SustainLevel = Asustain;
            aenv.ReleaseSamples = Arelease;

            fenv.AttackSamples = Fattack;
            fenv.DecaySamples = Fdecay;
            fenv.SustainLevel = Fsustain;
            fenv.ReleaseSamples = Frelease;
        }

        inline float Process(bool gate)
        {
            fenv.Process(gate);

            float output = 0;
            for (int i=0; i<COUNT; i++)
            {
                float hz = PitchHz * (1 + OFFSETS[i] * PSpread);
                uint32_t inc = Modules::Wavetable::GetPhaseIncrement(hz);
                
                phasor[i] += inc;
                float v = (phasor[i] - 0x7FFFFFFF) * 4.6566128e-10f;
                output += v * volumes[i];

                if (i == 3)
                {
                    phasorSub += (inc>>1);
                    float v = (phasorSub - 0x7FFFFFFF) * 4.6566128e-10f;
                    output += v * subOscGain;
                }
            }

            output *= gainAdjust;
            output = biq.Process(output); // high pass to remove aliasing below fundamental
            output += 0.2 * Modules::Wavetable::Sin(phasor[3]) * (1 + subOscGain); // increase fundamental frequency to compensate for high pass
            output += 0.35 * Modules::Wavetable::Sin(phasorSub) * subOscGain;

            output = filter.ProcessSample(output);
            output *= aenv.Process(gate);
            return output;
        }
    };

    class Superwave : public Generator
    {
        const static int VOICECOUNT = 6;

        const char* Tabs[4] = {"OSC", "FILT", "FENV", "AENV"};
        Menu menu;
        bool gate;
        int activeVoice = 0;
        SuperwaveVoice Voices[VOICECOUNT];

    public:
        Superwave();
        virtual const char** GetTabs() override { return Tabs; }
        virtual Menu* GetMenu() override { return &menu; }
        virtual int GetModSlots() override { return 18; }
        virtual void GetModSlotName(int idx, char* dest) override { strcpy(dest, menu.Captions[idx]); }
        virtual int ResolveSlot(int knobIdx) override { return menu.TopItem + knobIdx; }

        virtual inline void SetTab(int tab) override;
        virtual void Process(GeneratorArgs args) override;
        void UpdateVoices(GeneratorArgs args);
        
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Superwave";
            info.GeneratorId = "GNA-Superwave";
            info.Info = "Supersaw oscillator with tunable parameters.";
            info.Version = 1000;
            return info;
        }

        inline static void SplashScreen(U8G2* display)
        {
            display->clearDisplay();
            display->setDrawColor(1);
            display->setFont(BIG_FONT);
            display->setCursor(4, 36);
            display->print("Superwave");
        }
    };

}