#include "voices.h"
#include "logging.h"

namespace Cyber
{
    enum MenuItems
    {
        _InGain = 0,
        _OutGain,
        _StereoProcessing,
        _PitchOffset,
        _AmpControl,
        _LoadPreset,
        _SavePreset,
        _InitVoice,
        _ClearAllMods,
        _MidiCh,
        _AudioInL,
        _AudioInR,
        _AudioOutL,
        _AudioOutR,
        _CVIn,
        _GateIn,
    };

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
        args.Stereo = StereoProcessing;
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
        menu.Captions[_InGain] = "In Gain";
        menu.Captions[_OutGain] = "Out Gain";
        menu.Captions[_StereoProcessing] = "Stereo";
        menu.Captions[_PitchOffset] = "Pitch Offset";
        menu.Captions[_AmpControl] = "Amp Control";
        menu.Captions[_LoadPreset] = "> Load Preset";
        menu.Captions[_SavePreset] = "> Save Preset";
        menu.Captions[_InitVoice] = "> Init Voice";
        menu.Captions[_ClearAllMods] = "> Clear All Mods";
        menu.Captions[_MidiCh] = "Midi Ch";
        menu.Captions[_AudioInL] = "Audio In L";
        menu.Captions[_AudioInR] = "Audio In R";
        menu.Captions[_AudioOutL] = "Audio Out L";
        menu.Captions[_AudioOutR] = "Audio Out R";
        menu.Captions[_CVIn] = "CV In";
        menu.Captions[_GateIn] = "Gate In";

        menu.Values[_AudioInL] = 0.3;
        menu.Values[_AudioInR] = 0.3;
        menu.Values[_StereoProcessing] = 0.0;
        menu.Values[_AudioOutL] = 0.3;
        menu.Values[_AudioOutR] = 0.3;
        menu.Values[_CVIn] = 0.3;
        menu.Values[_GateIn] = 0.3;

        menu.Min[_PitchOffset] = -36;

        menu.Steps[_InGain] = 24;
        menu.Steps[_OutGain] = 24;
        menu.Steps[_StereoProcessing] = 1;
        menu.Steps[_PitchOffset] = 36*2;
        menu.Steps[_AmpControl] = 2;
        menu.Steps[_MidiCh] = 16+1;
        menu.Steps[_AudioInL] = 4;
        menu.Steps[_AudioInR] = 4;
        menu.Steps[_AudioOutL] = 4;
        menu.Steps[_AudioOutR] = 4;
        menu.Steps[_CVIn] = 4;
        menu.Steps[_GateIn] = 4;


        auto inChannelFormatter = [this](int idx, float val, int sv, char* dest)
        {
            if (sv == 0)
            {
                strcpy(dest, "Off");
                return;
            }

            if (idx == _AudioInR && !StereoProcessing)
                strcpy(dest, "--");
            else if (idx == _AudioOutR && !StereoProcessing)
                strcpy(dest, "--");
            else 
                sprintf(dest, "%d", sv);
        };

        menu.Formatters[_InGain] = [this](int idx, float v, int sv, char* s) { sprintf(s, "%.1fdB", -12.f + sv); };
        menu.Formatters[_OutGain] = [this](int idx, float v, int sv, char* s) { sprintf(s, "%.1fdB", -12.f + sv); };
        menu.Formatters[_StereoProcessing] = [this](int idx, float v, int sv, char* s) { strcpy(s, sv == 1 ? "Active" : "Disabled"); };
        menu.Formatters[_AmpControl] = [this](int idx, float v, int sv, char* s) 
        { 
            if (sv == 0) strcpy(s, "Off");
            else if (sv == 1) strcpy(s, "Gate");
            else if (sv == 2) strcpy(s, "Env 1");
        };
        menu.Formatters[_LoadPreset] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };
        menu.Formatters[_SavePreset] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };
        menu.Formatters[_InitVoice] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };
        menu.Formatters[_ClearAllMods] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };
        menu.Formatters[_MidiCh] = [this](int idx, float v, int sv, char* s) 
        { 
            if (sv == 0) strcpy(s, "Off");
            else if (sv == 1) strcpy(s, "Omni");
            else sprintf(s, "%d", sv-1); 
        };
        menu.Formatters[_AudioInL] = inChannelFormatter;
        menu.Formatters[_AudioInR] = inChannelFormatter;
        menu.Formatters[_AudioOutL] = inChannelFormatter;
        menu.Formatters[_AudioOutR] = inChannelFormatter;
        menu.Formatters[_CVIn] = inChannelFormatter;
        menu.Formatters[_GateIn] = inChannelFormatter;

        menu.ValueChangedCallback = [this](int idx, float val, int sv)
        {
            if (idx == _InGain) InGain = Utils::DB2Gainf(-12.0f + val * 24.f);
            if (idx == _OutGain) OutGain = Utils::DB2Gainf(-12.0f + val * 24.f);
            if (idx == _StereoProcessing) StereoProcessing = sv == 1;
            if (idx == _PitchOffset) PitchOffset = sv;
            if (idx == _AmpControl) AmpControl = sv;
            if (idx == _MidiCh) MidiChannel = sv - 2; // -2 = Off, -1 = Omni, 0-15 = channel
            if (idx == _AudioInL) AudioInLeft = sv - 1; // -1 = off, 0-3 = channel
            if (idx == _AudioInR) AudioInRight = sv - 1;
            if (idx == _AudioOutL) AudioOutLeft = sv - 1;
            if (idx == _AudioOutR) AudioOutRight = sv - 1;
            if (idx == _CVIn) CvIn = sv - 1;
            if (idx == _GateIn) GateIn = sv - 1;
        };

        menu.SetLength(15);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = true;
        menu.QuadMode = false;
    }
}

