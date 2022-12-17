#include "superwave.h"
#include "menu.h"
#include "logging.h"

namespace Cyber
{
    enum MenuItems
    {
        SEMI = 0,
        PSPREAD,
        VSPREAD,
        CENT,
        SUBOSC,
        STEREO,
        
        CUTOFF,
        RESO,
        DRIVE,
        ENVAMT,

        FATTACK,
        FDECAY,
        FSUSTAIN,
        FRELEASE,

        AATTACK,
        ADECAY,
        ASUSTAIN,
        ARELEASE,
    };

    Superwave::Superwave()
    {
        menu.Captions[SEMI] = "Semi";
        menu.Captions[PSPREAD] = "Spread";
        menu.Captions[VSPREAD] = "Mix";
        menu.Captions[CENT] = "Cent";
        menu.Captions[SUBOSC] = "Sub Osc";
        menu.Captions[STEREO] = "Stereo";

        menu.Captions[CUTOFF] = "Cutoff";
        menu.Captions[RESO] = "Reso";
        menu.Captions[DRIVE] = "Drive";
        menu.Captions[ENVAMT] = "EnvAmt";

        menu.Captions[FATTACK] = "Attack";
        menu.Captions[FDECAY] = "Decay";
        menu.Captions[FSUSTAIN] = "Sustain";
        menu.Captions[FRELEASE] = "Release";

        menu.Captions[AATTACK] = "Attack";
        menu.Captions[ADECAY] = "Decay";
        menu.Captions[ASUSTAIN] = "Sustain";
        menu.Captions[ARELEASE] = "Release";

        menu.Min[SEMI] = -36;
        menu.Min[CENT] = -50;
        menu.Steps[SEMI] = 72+1;
        menu.Steps[CENT] = 100+1;
        menu.Steps[STEREO] = 2;

        menu.Values[SEMI] = 0.5f;
        menu.Values[PSPREAD] = 0.5f;
        menu.Values[VSPREAD] = 0.5f;
        menu.Values[CENT] = 0.5f;
        menu.Values[CUTOFF] = 1.0f;
        menu.Values[DRIVE] = 0.5f;
        menu.Values[ENVAMT] = 0.5f;
        menu.Values[FSUSTAIN] = 0.0f;
        menu.Values[ASUSTAIN] = 1.0f;

        menu.Formatters[STEREO] = [](int idx, float val, int sv, char* dest) { strcpy(dest, sv == 1 ? "Stereo" : "Mono"); };

        menu.AddSectionBreak(STEREO);
        menu.AddSectionBreak(ENVAMT);
        menu.AddSectionBreak(FRELEASE);

        menu.SetLength(18);
        menu.QuadMode = true;
    }

    void Superwave::GetTab(int idx, char* dest)
    {
        strcpy(dest, Tabs[idx]);
    }

    void Superwave::SetTab(int tab)
    { 
        if (tab == ActiveTab)
            return;

        if (tab == 0)
            menu.SetSelectedItem(SEMI);
        else if (tab == 1)
            menu.SetSelectedItem(CUTOFF);
        else if (tab == 2)
            menu.SetSelectedItem(FATTACK);
        else if (tab == 3)
            menu.SetSelectedItem(AATTACK);

        ActiveTab = tab;
    }

    void Superwave::Process(GeneratorArgs args)
    {
        bool newGate = args.Data->Gate[0][0];
        bool trigger = !gate && newGate;
        bool stereo = menu.GetScaledValue(STEREO);
        if (trigger)
            activeVoice = (activeVoice + 1 + stereo) % VOICECOUNT; // jump 2 voices if stereo

        UpdateVoices(args);

        // figures out which voice is the most recently activated, taking into account stereo activates 2 adjacent voices
        bool active0 = activeVoice == 0 || (activeVoice == 5 && stereo);
        bool active1 = activeVoice == 1 || (activeVoice == 0 && stereo);
        bool active2 = activeVoice == 2 || (activeVoice == 1 && stereo);
        bool active3 = activeVoice == 3 || (activeVoice == 2 && stereo);
        bool active4 = activeVoice == 4 || (activeVoice == 3 && stereo);
        bool active5 = activeVoice == 5 || (activeVoice == 4 && stereo);

        for (int i = 0; i < args.Size; i++)
        {
            float output1 = 0.0f;
            float output2 = 0.0f;
            output1 += Voices[0].Process(newGate && active0);
            output2 += Voices[1].Process(newGate && active1);
            output1 += Voices[2].Process(newGate && active2);
            output2 += Voices[3].Process(newGate && active3);
            output1 += Voices[4].Process(newGate && active4);
            output2 += Voices[5].Process(newGate && active5);

            if (stereo)
            {
                args.Data->Out[0][i] = output1 * 0.8;
                args.Data->Out[1][i] = output2 * 0.8;
            }
            else
            {
                float v = (output1 + output2) * 0.8;
                args.Data->Out[0][i] = v;
                args.Data->Out[1][i] = v;
            }
        }

        gate = newGate;
    }

    void Superwave::UpdateVoices(GeneratorArgs args)
    {
        bool stereo = menu.GetScaledValue(STEREO);
        int semi = menu.GetScaledValue(SEMI);
        int cent = menu.GetScaledValue(CENT);
        float pitchCv = args.GetModulationSlow(SEMI);

        float pitch = args.Data->Cv[0][0] * CV_RANGE * 12 + semi + cent * 0.01 + pitchCv * 24;
        float pitchHz = Utils::Note2HzLut(pitch);

        auto getValue = [this, args](int idx)
        {
            return Utils::Clamp(menu.Values[idx] + args.GetModulationSlow(idx));
        };

        for (int i = 0; i < VOICECOUNT; i++)
        {
            if (i == activeVoice)
            {
                Voices[i].PitchHz = pitchHz;
                if (stereo)
                    Voices[(i+1) % VOICECOUNT].PitchHz = pitchHz;
            }
            
            Voices[i].PSpread = getValue(PSPREAD);
            Voices[i].VSpread = getValue(VSPREAD);
            Voices[i].SubOsc = getValue(SUBOSC);

            Voices[i].Cutoff = getValue(CUTOFF);
            Voices[i].Resonance = getValue(RESO);
            Voices[i].Drive = getValue(DRIVE);
            Voices[i].EnvAmt = getValue(ENVAMT);
            
            Voices[i].Fattack = 10 + Utils::Resp3dec(getValue(FATTACK)) * SAMPLERATE * 20;
            Voices[i].Fdecay = 10 + Utils::Resp3dec(getValue(FDECAY)) * SAMPLERATE * 20;
            Voices[i].Fsustain = getValue(FSUSTAIN);
            Voices[i].Frelease = 10 + Utils::Resp3dec(getValue(FRELEASE)) * SAMPLERATE * 20;

            Voices[i].Aattack = 10 + Utils::Resp3dec(getValue(AATTACK)) * SAMPLERATE * 20;
            Voices[i].Adecay = 10 + Utils::Resp3dec(getValue(ADECAY)) * SAMPLERATE * 20;
            Voices[i].Asustain = getValue(ASUSTAIN);
            Voices[i].Arelease = 10 + Utils::Resp3dec(getValue(ARELEASE)) * SAMPLERATE * 20;
            
            Voices[i].Update();
        }
    }
}
