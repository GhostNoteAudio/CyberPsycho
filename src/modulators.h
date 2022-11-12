#pragma once
#include "constants.h"
#include "modules/envelope.h"
#include "modules/lfo.h"
#include "stdint.h"
#include "generator.h"

namespace Cyber
{
    class Modulators
    {
        Menu menu;
        Modules::Envelope env1;
        Modules::Envelope env2;
        Modules::Lfo lfo1;
        Modules::Lfo lfo2;

    public:
        float OutEnv1[BUFFER_SIZE];
        float OutEnv2[BUFFER_SIZE];
        float OutLfo1[BUFFER_SIZE];
        float OutLfo2[BUFFER_SIZE];

        Modulators()
        {
            env1.Mode = Modules::Envelope::EnvMode::ADSR;
            env1.ReleaseCurve = Modules::Envelope::EnvCurve::Exp;
            env1.AttackSamples = 40000;
            env1.DecaySamples = 10000;
            env1.SustainLevel = 0.5;
            env1.ReleaseSamples = 80000;

            env2.AttackSamples = 1000;
            env2.ReleaseSamples = 80000;

            env1.UpdateParams();
            env2.UpdateParams();

            InitMenu();
        }

        void InitMenu()
        {
            menu.Captions[0] = "Attack";
            menu.Captions[1] = "Decay";
            menu.Captions[2] = "Sustain";
            menu.Captions[3] = "Release";

            menu.Captions[4] = "Attack";
            menu.Captions[5] = "Decay";
            menu.Captions[6] = "Sustain";
            menu.Captions[7] = "Release";

            menu.Captions[8] = "Freq";
            menu.Captions[9] = "Shape";
            menu.Captions[10] = "Retrigger";
            menu.Captions[11] = "Polarity";

            menu.Captions[12] = "Freq";
            menu.Captions[13] = "Shape";
            menu.Captions[14] = "Retrigger";
            menu.Captions[15] = "Polarity";

            menu.Captions[16] = "Env1 Atk";
            menu.Captions[17] = "Env2 Atk";
            menu.Captions[18] = "Env1 Rel";
            menu.Captions[19] = "Env2 Rel";

            menu.Max[0] = 1023;
            menu.Max[1] = 1023;
            menu.Max[2] = 1023;
            menu.Max[3] = 1023;

            menu.Max[4] = 1023;
            menu.Max[5] = 1023;
            menu.Max[6] = 1023;
            menu.Max[7] = 1023;

            menu.Max[8] = 1023;
            menu.Max[9] = 4;
            menu.Max[10] = 1;
            menu.Max[11] = 1;

            menu.Max[12] = 1023;
            menu.Max[13] = 4;
            menu.Max[14] = 1;
            menu.Max[15] = 1;

            menu.Max[16] = 1;
            menu.Max[17] = 1;
            menu.Max[18] = 1;
            menu.Max[19] = 1;

            menu.Values[0] = 50;
            menu.Values[1] = 200;
            menu.Values[2] = 900;
            menu.Values[3] = 300;

            menu.Values[4] = 50;
            menu.Values[5] = 200;
            menu.Values[6] = 900;
            menu.Values[7] = 300;

            menu.Values[8] = 50;
            menu.Values[9] = 0;
            menu.Values[10] = 0;
            menu.Values[11] = 0;

            menu.Values[12] = 50;
            menu.Values[13] = 0;
            menu.Values[14] = 0;
            menu.Values[15] = 0;

            menu.Values[16] = 0;
            menu.Values[17] = 0;
            menu.Values[18] = 1;
            menu.Values[19] = 1;

            auto formatLin = [this](int idx, int16_t val, char* dest){ sprintf(dest, "%.0f%%", 100.0f * val / (float)menu.Max[idx]); };
            auto formatEnvTime = [this](int idx, int16_t val, char* dest)
            {
                float sec = Utils::Resp3dec(val / 1023.0f) * 20;
                if (sec <= 1)
                    sprintf(dest, "%.3fs", sec);
                else if (sec <= 10)
                    sprintf(dest, "%.2fs", sec);
                else
                    sprintf(dest, "%.1fs", sec);
            };
            auto formatFreq = [this](int idx, int16_t val, char* dest)
            { 
                float f = Utils::Resp4dec(val / 1023.0f) * 200;
                if (f < 1)
                    sprintf(dest, "%.3fHz", f);
                else if (f < 10)
                    sprintf(dest, "%.2fHz", f);
                else if (f < 100)
                    sprintf(dest, "%.1fHz", f);
                else
                    sprintf(dest, "%.0fHz", f);
            };
            auto formatOnOff = [this](int idx, int16_t val, char* dest){ val == 0 ? strcpy(dest, "Off") : strcpy(dest, "On"); };
            auto formatPolarity = [this](int idx, int16_t val, char* dest){ val == 0 ? strcpy(dest, "Bipolar") : strcpy(dest, "Unipolar"); };
            auto formatShape = [this](int idx, int16_t val, char* dest){ strcpy(dest, Modules::Lfo::GetShapeName(val)); };
            auto formatEnvShape = [this](int idx, int16_t val, char* dest){ val == 0 ? strcpy(dest, "Lin") : strcpy(dest, "Exp"); };

            menu.Formatters[0] = formatEnvTime;
            menu.Formatters[1] = formatEnvTime;
            menu.Formatters[2] = formatLin;
            menu.Formatters[3] = formatEnvTime;

            menu.Formatters[4] = formatEnvTime;
            menu.Formatters[5] = formatEnvTime;
            menu.Formatters[6] = formatLin;
            menu.Formatters[7] = formatEnvTime;

            menu.Formatters[8] = formatFreq;
            menu.Formatters[9] = formatShape;
            menu.Formatters[10] = formatOnOff;
            menu.Formatters[11] = formatPolarity;

            menu.Formatters[12] = formatFreq;
            menu.Formatters[13] = formatShape;
            menu.Formatters[14] = formatOnOff;
            menu.Formatters[15] = formatPolarity;

            menu.Formatters[16] = formatEnvShape;
            menu.Formatters[17] = formatEnvShape;
            menu.Formatters[18] = formatEnvShape;
            menu.Formatters[19] = formatEnvShape;

            menu.ValueChangedCallback = [this](int idx, int16_t val)
            {
                if (idx == 0) env1.AttackSamples = Utils::Resp3dec(val / 1023.0f) * 20 * SAMPLERATE;
                if (idx == 1) env1.DecaySamples = Utils::Resp3dec(val / 1023.0f) * 20 * SAMPLERATE;
                if (idx == 2) env1.SustainLevel = val / 1023.0f;
                if (idx == 3) env1.ReleaseSamples = Utils::Resp3dec(val / 1023.0f) * 20 * SAMPLERATE;

                if (idx == 4) env2.AttackSamples = Utils::Resp3dec(val / 1023.0f) * 20 * SAMPLERATE;
                if (idx == 5) env2.DecaySamples = Utils::Resp3dec(val / 1023.0f) * 20 * SAMPLERATE;
                if (idx == 6) env2.SustainLevel = val / 1023.0f;
                if (idx == 7) env2.ReleaseSamples = Utils::Resp3dec(val / 1023.0f) * 20 * SAMPLERATE;

                if (idx == 8) lfo1.Frequency = Utils::Resp4dec(val / 1023.0f) * 200;
                if (idx == 9) lfo1.Waveshape = (Modules::Lfo::Shape)val;
                if (idx == 10) lfo1.Retrigger = !(val == 0);
                if (idx == 11) lfo1.Unipolar = val == 1;

                if (idx == 12) lfo2.Frequency = Utils::Resp4dec(val / 1023.0f) * 200;
                if (idx == 13) lfo2.Waveshape = (Modules::Lfo::Shape)val;
                if (idx == 14) lfo2.Retrigger = !(val == 0);
                if (idx == 15) lfo2.Unipolar = val == 1;

                if (idx == 16) env1.AttackCurve = (val == 0) ? Modules::Envelope::EnvCurve::Linear : Modules::Envelope::EnvCurve::Exp;
                if (idx == 17) env2.AttackCurve = (val == 0) ? Modules::Envelope::EnvCurve::Linear : Modules::Envelope::EnvCurve::Exp;
                if (idx == 18) env1.ReleaseCurve = env1.DecayCurve = (val == 0) ? Modules::Envelope::EnvCurve::Linear : Modules::Envelope::EnvCurve::Exp;
                if (idx == 19) env2.ReleaseCurve = env2.DecayCurve = (val == 0) ? Modules::Envelope::EnvCurve::Linear : Modules::Envelope::EnvCurve::Exp;
            };
            
            menu.SetLength(20);
            menu.SelectedItem = 0;
            menu.TopItem = 0;
            menu.EnableSelection = false;
            menu.QuadMode = true;
        }

        Menu* GetMenu() { return &menu; }

        void Process(GeneratorArgs args)
        {
            for (int i = 0; i < args.Size; i++)
            {
                bool g = args.Gate[i];
                OutEnv1[i] = env1.Process(g);
                OutEnv2[i] = env2.Process(g);
                OutLfo1[i] = 1+lfo1.Process(g);
                OutLfo2[i] = 1+lfo2.Process(g);
            }
        }
    };
}
