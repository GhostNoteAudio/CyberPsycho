#include "voices.h"

namespace Cyber
{
    namespace Voices
    {
        Voice Voices[4];
        uint8_t ActiveVoice = 0;
    }

    void Voice::Process(FpBuffer* fpData)
    {
        float temp1L[BUFFER_SIZE];
        float temp1R[BUFFER_SIZE];
        float temp2L[BUFFER_SIZE];
        float temp2R[BUFFER_SIZE];
        bool gate[BUFFER_SIZE];
        float cv[BUFFER_SIZE];
        float emptyBuf[BUFFER_SIZE];

        Utils::ZeroBuffer(emptyBuf, BUFFER_SIZE);
        Utils::ZeroBuffer(temp1R, BUFFER_SIZE);
        
        if (GateIn == -1)
            Utils::ZeroBuffer(gate, BUFFER_SIZE);
        else
            Utils::Copy(gate, fpData->Gate[GateIn], BUFFER_SIZE);

        if (CvIn == -1)
            Utils::ZeroBuffer(cv, BUFFER_SIZE);
        else
            Utils::Copy(cv, fpData->Cv[CvIn], BUFFER_SIZE);

        matrix.modulators = &modulators;
        matrix.fpData = fpData;

        GeneratorArgs args;
        args.Bpm = 120;
        args.Gate = gate;
        args.Cv = cv;
        modulators.GetModulationFast = [this](ModDest dest, uint8_t slot) { return matrix.GetModulationFast(dest, slot); };
        modulators.GetModulationSlow = [this](ModDest dest, uint8_t slot) { return matrix.GetModulationSlow(dest, slot); };
        modulators.Process(args);
        
        args.GetModulationFast = [this](uint8_t slot) { return matrix.GetModulationFast(Cyber::ModDest::Generator, slot); };
        args.GetModulationSlow = [this](uint8_t slot) { return matrix.GetModulationSlow(Cyber::ModDest::Generator, slot); };
        args.InputLeft = AudioInLeft == -1 ? emptyBuf : fpData->Mod[AudioInLeft];
        args.InputRight = AudioInRight == -1 ? emptyBuf : fpData->Mod[AudioInRight];
        args.OutputLeft = temp1L;
        args.OutputRight = temp1R;
        Gen->Process(args);

        args.GetModulationFast = [this](uint8_t slot) { return matrix.GetModulationFast(Cyber::ModDest::Insert1, slot); };
        args.GetModulationSlow = [this](uint8_t slot) { return matrix.GetModulationSlow(Cyber::ModDest::Insert1, slot); };
        args.InputLeft = temp1L;
        args.InputRight = temp1R;
        args.OutputLeft = temp2L;
        args.OutputRight = temp2R;
        Inserts[0]->Process(args);

        args.GetModulationFast = [this](uint8_t slot) { return matrix.GetModulationFast(Cyber::ModDest::Insert2, slot); };
        args.GetModulationSlow = [this](uint8_t slot) { return matrix.GetModulationSlow(Cyber::ModDest::Insert2, slot); };
        args.InputLeft = temp2L;
        args.InputRight = temp2R;
        args.OutputLeft = temp1L;
        args.OutputRight = temp1R;
        Inserts[1]->Process(args);

        args.GetModulationFast = [this](uint8_t slot) { return matrix.GetModulationFast(Cyber::ModDest::Insert3, slot); };
        args.GetModulationSlow = [this](uint8_t slot) { return matrix.GetModulationSlow(Cyber::ModDest::Insert3, slot); };
        args.InputLeft = temp1L;
        args.InputRight = temp1R;
        args.OutputLeft = temp2L;
        args.OutputRight = temp2R;
        Inserts[2]->Process(args);

        args.GetModulationFast = [this](uint8_t slot) { return matrix.GetModulationFast(Cyber::ModDest::Insert4, slot); };
        args.GetModulationSlow = [this](uint8_t slot) { return matrix.GetModulationSlow(Cyber::ModDest::Insert4, slot); };
        args.InputLeft = temp2L;
        args.InputRight = temp2R;
        args.OutputLeft = temp1L;
        args.OutputRight = temp1R;
        Inserts[3]->Process(args);

        if (AmpControl == 1)
        {
            Utils::Multiply(temp1L, gate, args.Size);
            Utils::Multiply(temp1R, gate, args.Size);
        }
        else if (AmpControl == 2)
        {
            Utils::Multiply(temp1L, modulators.OutEnv1, args.Size);
            Utils::Multiply(temp1R, modulators.OutEnv1, args.Size);
        }
        //Utils::Multiply(temp1, modulators.OutLfo1, args.Size);

        if (AudioOutLeft != -1)
            Utils::Mix(fpData->Out[AudioOutLeft], temp1L, OutGain, args.Size);
        if (AudioOutRight != -1)
            Utils::Mix(fpData->Out[AudioOutRight], temp1R, OutGain, args.Size);
    }

    void Voice::InitMenu()
    {
        menu.Captions[0] = "In Gain";
        menu.Captions[1] = "Out Gain";
        menu.Captions[2] = "Pitch Offset";
        menu.Captions[3] = "Amp Control";
        menu.Captions[4] = "> Load Preset";
        menu.Captions[5] = "> Save Preset";
        menu.Captions[6] = "> Init Voice";
        menu.Captions[7] = "> Clear All Mods";
        menu.Captions[8] = "Midi Ch";
        menu.Captions[9] = "Audio In L";
        menu.Captions[10] = "Audio In R";
        menu.Captions[11] = "Audio Out L";
        menu.Captions[12] = "Audio Out R";
        menu.Captions[13] = "CV In";
        menu.Captions[14] = "Gate In";

        menu.Values[9] = 1;
        menu.Values[10] = 1;
        menu.Values[11] = 1;
        menu.Values[12] = 1;
        menu.Values[13] = 1;
        menu.Values[14] = 1;

        menu.Max[0] = 48;
        menu.Max[1] = 48;
        menu.Max[2] = 36;
        menu.Max[3] = 2;
        menu.Max[8] = 17;
        menu.Max[9] = 4;
        menu.Max[10] = 4;
        menu.Max[11] = 4;
        menu.Max[12] = 4;
        menu.Max[13] = 4;
        menu.Max[14] = 4;

        menu.Min[2] = -36;

        auto inChannelFormatter = [](int idx, int16_t val, char* dest)
        {
            if (val == 0) strcpy(dest, "Off");
            else sprintf(dest, "%d", val);
        };

        menu.Formatters[0] = [](int idx, int16_t v, char* s) { sprintf(s, "%.1fdB", -12+v*0.5f); };
        menu.Formatters[1] = [](int idx, int16_t v, char* s) { sprintf(s, "%.1fdB", -12+v*0.5f); };
        menu.Formatters[3] = [](int idx, int16_t v, char* s) 
        { 
            if (v == 0) strcpy(s, "Off");
            else if (v == 1) strcpy(s, "Gate");
            else if (v == 2) strcpy(s, "Env 1");
        };
        menu.Formatters[4] = [](int idx, int16_t v, char* s) { strcpy(s, ""); };
        menu.Formatters[5] = [](int idx, int16_t v, char* s) { strcpy(s, ""); };
        menu.Formatters[6] = [](int idx, int16_t v, char* s) { strcpy(s, ""); };
        menu.Formatters[7] = [](int idx, int16_t v, char* s) { strcpy(s, ""); };
        menu.Formatters[8] = [](int idx, int16_t v, char* s) 
        { 
            if (v == 0) strcpy(s, "Off");
            else if (v == 1) strcpy(s, "Omni");
            else sprintf(s, "%d", v-1); 
        };
        menu.Formatters[9] = inChannelFormatter;
        menu.Formatters[10] = inChannelFormatter;
        menu.Formatters[11] = inChannelFormatter;
        menu.Formatters[12] = inChannelFormatter;
        menu.Formatters[13] = inChannelFormatter;
        menu.Formatters[14] = inChannelFormatter;

        menu.ValueChangedCallback = [this](int idx, int16_t val)
        {
            if (idx == 0) InGain = Utils::DB2Gainf(-12.0f + val * 0.5f);
            if (idx == 1) OutGain = Utils::DB2Gainf(-12.0f + val * 0.5f);
            if (idx == 2) PitchOffset = val;
            if (idx == 3) AmpControl = val;
            if (idx == 8) MidiChannel = val - 2; // -2 = Off, -1 = Omni, 0-15 = channel
            if (idx == 9) AudioInLeft = val - 1; // -1 = off, 0-3 = channel
            if (idx == 10) AudioInRight = val - 1;
            if (idx == 11) AudioOutLeft = val - 1;
            if (idx == 12) AudioOutRight = val - 1;
            if (idx == 13) CvIn = val - 1;
            if (idx == 14) GateIn = val - 1;
        };

        menu.SetLength(15);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = true;
        menu.QuadMode = false;
    }
}

