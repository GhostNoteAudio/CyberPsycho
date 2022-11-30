#include "mod_matrix.h"
#include "logging.h"
#include "voices.h"

namespace Cyber
{
    void ModMatrix::InitMenu()
    {
        /*
        ModSource Source;
        ModDest Dest;
        uint8_t Slot;
        float Amount;
        */
        menu.QuadMode = true;
        menu.EnableSelection = false;
        menu.SetLength(4);

        menu.RenderCustomDisplayCallback = [this](U8G2* display)
        {
            int offset = (this->activeRoute+1) < 10 ? 0 : 3;

            display->setCursor(56-offset, 10);
            display->print("[");
            display->setCursor(63-offset, 10);
            display->print(this->activeRoute+1);
            display->setCursor(70+offset, 10);
            display->print("]");
        };

        menu.Formatters[0] = [](int idx, int16_t val, char* dest) 
        {
                 if (val == (int)ModSource::Off) strcpy(dest, "Off");
            else if (val == (int)ModSource::Mod1) strcpy(dest, "Mod 1");
            else if (val == (int)ModSource::Mod2) strcpy(dest, "Mod 2");
            else if (val == (int)ModSource::Mod3) strcpy(dest, "Mod 3");
            else if (val == (int)ModSource::Mod4) strcpy(dest, "Mod 4");
            else if (val == (int)ModSource::Cv1) strcpy(dest, "CV 1");
            else if (val == (int)ModSource::Cv2) strcpy(dest, "CV 2");
            else if (val == (int)ModSource::Cv3) strcpy(dest, "CV 3");
            else if (val == (int)ModSource::Cv4) strcpy(dest, "CV 4");
            else if (val == (int)ModSource::Gate1) strcpy(dest, "Gate 1");
            else if (val == (int)ModSource::Gate2) strcpy(dest, "Gate 2");
            else if (val == (int)ModSource::Gate3) strcpy(dest, "Gate 3");
            else if (val == (int)ModSource::Gate4) strcpy(dest, "Gate 4");
            else if (val == (int)ModSource::Env1) strcpy(dest, "Env 1");
            else if (val == (int)ModSource::Env2) strcpy(dest, "Env 2");
            else if (val == (int)ModSource::Lfo1) strcpy(dest, "Lfo 1");
            else if (val == (int)ModSource::Lfo2) strcpy(dest, "Lfo 2");
        };

        menu.Formatters[1] = [](int idx, int16_t val, char* dest) 
        {
                 if (val == (int)ModDest::Voice) strcpy(dest, "Voice");
            else if (val == (int)ModDest::Generator) strcpy(dest, "Generator");
            else if (val == (int)ModDest::Insert1) strcpy(dest, "Insert 1");
            else if (val == (int)ModDest::Insert2) strcpy(dest, "Insert 2");
            else if (val == (int)ModDest::Insert3) strcpy(dest, "Insert 3");
            else if (val == (int)ModDest::Insert4) strcpy(dest, "Insert 4");
            else if (val == (int)ModDest::Env1) strcpy(dest, "Env 1");
            else if (val == (int)ModDest::Env2) strcpy(dest, "Env 2");
            else if (val == (int)ModDest::Lfo1) strcpy(dest, "Lfo 1");
            else if (val == (int)ModDest::Lfo2) strcpy(dest, "Lfo 2");
            else if (val == (int)ModDest::ModMatrix) strcpy(dest, "Matrix");
        };

        menu.Formatters[2] = [this](int idx, int16_t val, char* dest) 
        {
            if (this->Routes[activeRoute].Dest == ModDest::Voice)
                strcpy(dest, voice->GetModLabel(val));
            else if (this->Routes[activeRoute].Dest == ModDest::ModMatrix)
                strcpy(dest, this->GetModLabel(val));
            else if (this->Routes[activeRoute].Dest == ModDest::Env1)
                strcpy(dest, modulators->GetModLabel(ModDest::Env1, val));
            else if (this->Routes[activeRoute].Dest == ModDest::Env2)
                strcpy(dest, modulators->GetModLabel(ModDest::Env2, val));
            else if (this->Routes[activeRoute].Dest == ModDest::Lfo1)
                strcpy(dest, modulators->GetModLabel(ModDest::Lfo1, val));
            else if (this->Routes[activeRoute].Dest == ModDest::Lfo2)
                strcpy(dest, modulators->GetModLabel(ModDest::Lfo2, val));
            else
                sprintf(dest, "%d", val);
        };

        menu.Formatters[3] = [](int idx, int16_t val, char* dest) { sprintf(dest, "%.1f%%", val / 1023.0f * 100.0f); };

        menu.Max[0] = 15;
        menu.Max[1] = 10;
        menu.Max[2] = 15;
        menu.Max[3] = 1023;
        menu.Min[3] = -1023;
        menu.Captions[0] = "Source";
        menu.Captions[1] = "Destination";
        menu.Captions[2] = "Slot";
        menu.Captions[3] = "Amount";

        menu.HandleEncoderCallback = [this](Menu* menu, int tick)
        {
            LogInfof("Tick: %d", tick);
            if (tick < 0 && this->activeRoute == 0)
                return;
            if (tick > 0 && this->activeRoute == ModRouteCount-1)
                return;
            this->activeRoute += tick;
            LogInfof("Active Route: %d", this->activeRoute);

            menu->Values[0] = (int)this->Routes[activeRoute].Source;
            menu->Values[1] = (int)this->Routes[activeRoute].Dest;
            menu->Values[2] = this->Routes[activeRoute].Slot;
            menu->Values[3] = this->Routes[activeRoute].AmountRaw;
        };

        menu.HandlePotCallback = [this](Menu* menu, int idx, float value)
        {
            menu->SetValueF(idx, value);
        };

        menu.ValueChangedCallback = [this](int idx, int16_t val)
        {
            if (idx == 0) this->Routes[activeRoute].Source = (ModSource)val;
            if (idx == 1) this->Routes[activeRoute].Dest = (ModDest)val;
            if (idx == 2) this->Routes[activeRoute].Slot = val;
            if (idx == 3)
            {
                this->Routes[activeRoute].AmountRaw = val;
                this->Routes[activeRoute].Amount = val / 1023.0f;
            }
        };
    }
}