#include "voices.h"
#include "logging.h"

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

        Utils::Mix(args.Cv, matrix.GetModulationFast(ModDest::Voice, 0), 1.0f, BUFFER_SIZE);
        
        args.GetModulationFast = [this](uint8_t slot) { return matrix.GetModulationFast(Cyber::ModDest::Generator, slot); };
        args.GetModulationSlow = [this](uint8_t slot) { return matrix.GetModulationSlow(Cyber::ModDest::Generator, slot); };
        args.InputLeft = AudioInLeft == -1 ? emptyBuf : fpData->Mod[AudioInLeft];
        args.InputRight = AudioInRight == -1 ? emptyBuf : fpData->Mod[AudioInRight];
        args.OutputLeft = temp1L;
        args.OutputRight = temp1R;
        Utils::Multiply1(args.InputLeft, matrix.GetModulationFast(ModDest::Voice, 1), BUFFER_SIZE);
        Utils::Multiply1(args.InputRight, matrix.GetModulationFast(ModDest::Voice, 1), BUFFER_SIZE);
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
        
        Utils::Multiply1(temp1L, matrix.GetModulationFast(ModDest::Voice, 2), BUFFER_SIZE);
        Utils::Multiply1(temp1R, matrix.GetModulationFast(ModDest::Voice, 2), BUFFER_SIZE);

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

        menu.Values[9] = 0.3;
        menu.Values[10] = 0.3;
        menu.Values[11] = 0.3;
        menu.Values[12] = 0.3;
        menu.Values[13] = 0.3;
        menu.Values[14] = 0.3;

        menu.Min[2] = -36;

        menu.Steps[0] = 24;
        menu.Steps[1] = 24;
        menu.Steps[2] = 36*2;
        menu.Steps[3] = 2;
        menu.Steps[8] = 16+1;
        menu.Steps[9] = 4;
        menu.Steps[10] = 4;
        menu.Steps[11] = 4;
        menu.Steps[12] = 4;
        menu.Steps[13] = 4;
        menu.Steps[14] = 4;


        auto inChannelFormatter = [this](int idx, float val, int sv, char* dest)
        {
            if (sv == 0) strcpy(dest, "Off");
            else sprintf(dest, "%d", sv);
        };

        menu.Formatters[0] = [this](int idx, float v, int sv, char* s) { sprintf(s, "%.1fdB", -12.f + sv); };
        menu.Formatters[1] = [this](int idx, float v, int sv, char* s) { sprintf(s, "%.1fdB", -12.f + sv); };
        menu.Formatters[3] = [this](int idx, float v, int sv, char* s) 
        { 
            if (sv == 0) strcpy(s, "Off");
            else if (sv == 1) strcpy(s, "Gate");
            else if (sv == 2) strcpy(s, "Env 1");
        };
        menu.Formatters[4] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };
        menu.Formatters[5] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };
        menu.Formatters[6] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };
        menu.Formatters[7] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };
        menu.Formatters[8] = [this](int idx, float v, int sv, char* s) 
        { 
            if (sv == 0) strcpy(s, "Off");
            else if (sv == 1) strcpy(s, "Omni");
            else sprintf(s, "%d", sv-1); 
        };
        menu.Formatters[9] = inChannelFormatter;
        menu.Formatters[10] = inChannelFormatter;
        menu.Formatters[11] = inChannelFormatter;
        menu.Formatters[12] = inChannelFormatter;
        menu.Formatters[13] = inChannelFormatter;
        menu.Formatters[14] = inChannelFormatter;

        menu.ValueChangedCallback = [this](int idx, float val, int sv)
        {
            if (idx == 0) InGain = Utils::DB2Gainf(-12.0f + val * 24.f);
            if (idx == 1) OutGain = Utils::DB2Gainf(-12.0f + val * 24.f);
            if (idx == 2) PitchOffset = sv;
            if (idx == 3) AmpControl = sv;
            if (idx == 8) MidiChannel = sv - 2; // -2 = Off, -1 = Omni, 0-15 = channel
            if (idx == 9) AudioInLeft = sv - 1; // -1 = off, 0-3 = channel
            if (idx == 10) AudioInRight = sv - 1;
            if (idx == 11) AudioOutLeft = sv - 1;
            if (idx == 12) AudioOutRight = sv - 1;
            if (idx == 13) CvIn = sv - 1;
            if (idx == 14) GateIn = sv - 1;
        };

        menu.SetLength(15);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = true;
        menu.QuadMode = false;
    }
}

