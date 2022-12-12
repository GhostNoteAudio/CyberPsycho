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

        menu.Formatters[0] = [this](int idx, float val, int sv, char* dest) 
        {
                 if (sv == (int)ModSource::Off) strcpy(dest, "Off");
            else if (sv == (int)ModSource::Mod1) strcpy(dest, "Mod 1");
            else if (sv == (int)ModSource::Mod2) strcpy(dest, "Mod 2");
            else if (sv == (int)ModSource::Mod3) strcpy(dest, "Mod 3");
            else if (sv == (int)ModSource::Mod4) strcpy(dest, "Mod 4");
            else if (sv == (int)ModSource::Cv1) strcpy(dest, "CV 1");
            else if (sv == (int)ModSource::Cv2) strcpy(dest, "CV 2");
            else if (sv == (int)ModSource::Cv3) strcpy(dest, "CV 3");
            else if (sv == (int)ModSource::Cv4) strcpy(dest, "CV 4");
            else if (sv == (int)ModSource::Gate1) strcpy(dest, "Gate 1");
            else if (sv == (int)ModSource::Gate2) strcpy(dest, "Gate 2");
            else if (sv == (int)ModSource::Gate3) strcpy(dest, "Gate 3");
            else if (sv == (int)ModSource::Gate4) strcpy(dest, "Gate 4");
            else if (sv == (int)ModSource::Env1) strcpy(dest, "Env 1");
            else if (sv == (int)ModSource::Env2) strcpy(dest, "Env 2");
            else if (sv == (int)ModSource::Lfo1) strcpy(dest, "Lfo 1");
            else if (sv == (int)ModSource::Lfo2) strcpy(dest, "Lfo 2");
        };

        menu.Formatters[1] = [this](int idx, float val, int sv, char* dest) 
        {
                 if (sv == (int)ModDest::Voice) strcpy(dest, "Voice");
            else if (sv == (int)ModDest::Generator) strcpy(dest, "Generator");
            else if (sv == (int)ModDest::Insert1) strcpy(dest, "Insert 1");
            else if (sv == (int)ModDest::Insert2) strcpy(dest, "Insert 2");
            else if (sv == (int)ModDest::Insert3) strcpy(dest, "Insert 3");
            else if (sv == (int)ModDest::Insert4) strcpy(dest, "Insert 4");
            else if (sv == (int)ModDest::Env1) strcpy(dest, "Env 1");
            else if (sv == (int)ModDest::Env2) strcpy(dest, "Env 2");
            else if (sv == (int)ModDest::Lfo1) strcpy(dest, "Lfo 1");
            else if (sv == (int)ModDest::Lfo2) strcpy(dest, "Lfo 2");
            else if (sv == (int)ModDest::ModMatrix) strcpy(dest, "Matrix");
        };

        menu.Formatters[2] = [this](int idx, float val, int sv, char* dest) 
        {
            if (this->Routes[activeRoute].Dest == ModDest::Voice)
                strcpy(dest, voice->GetModLabel(sv));
            else if (this->Routes[activeRoute].Dest == ModDest::ModMatrix)
                strcpy(dest, this->GetModLabel(sv));
            else if (this->Routes[activeRoute].Dest == ModDest::Env1)
                strcpy(dest, modulators->GetModLabel(ModDest::Env1, sv));
            else if (this->Routes[activeRoute].Dest == ModDest::Env2)
                strcpy(dest, modulators->GetModLabel(ModDest::Env2, sv));
            else if (this->Routes[activeRoute].Dest == ModDest::Lfo1)
                strcpy(dest, modulators->GetModLabel(ModDest::Lfo1, sv));
            else if (this->Routes[activeRoute].Dest == ModDest::Lfo2)
                strcpy(dest, modulators->GetModLabel(ModDest::Lfo2, sv));
            else if (this->Routes[activeRoute].Dest == ModDest::Generator)
            {
                auto menu = voice->Gen->GetMenu();
                strcpy(dest, sv < menu->GetLength() ? menu->Captions[sv] : "-");
            }
            else if (this->Routes[activeRoute].Dest == ModDest::Insert1)
            {
                auto menu = voice->Inserts[0]->GetMenu();
                strcpy(dest, sv < menu->GetLength() ? menu->Captions[sv] : "-");
            }
            else if (this->Routes[activeRoute].Dest == ModDest::Insert2)
            {
                auto menu = voice->Inserts[1]->GetMenu();
                strcpy(dest, sv < menu->GetLength() ? menu->Captions[sv] : "-");
            }
            else if (this->Routes[activeRoute].Dest == ModDest::Insert3)
            {
                auto menu = voice->Inserts[2]->GetMenu();
                strcpy(dest, sv < menu->GetLength() ? menu->Captions[sv] : "-");
            }
            else if (this->Routes[activeRoute].Dest == ModDest::Insert4)
            {
                auto menu = voice->Inserts[3]->GetMenu();
                strcpy(dest, sv < menu->GetLength() ? menu->Captions[sv] : "-");
            }
            else
                sprintf(dest, "%d", sv);
        };

        menu.Formatters[3] = [](int idx, float val, int sv, char* dest) { sprintf(dest, "%.0f%%", (val-0.5) * 200.0f); };

        menu.Steps[0] = 16;
        menu.Steps[1] = 11;
        menu.Steps[2] = 16;
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
            menu->Values[3] = this->Routes[activeRoute].Amount;
        };

        menu.HandlePotCallback = [](Menu* menu, int idx, float value)
        {
            menu->SetValue(idx, value);
        };

        menu.ValueChangedCallback = [this](int idx, float val, int sv)
        {
            if (idx == 0) this->Routes[activeRoute].Source = (ModSource)sv;
            if (idx == 1) this->Routes[activeRoute].Dest = (ModDest)sv;
            if (idx == 2) this->Routes[activeRoute].Slot = sv;
            if (idx == 3) this->Routes[activeRoute].Amount = val;
        };
    }
}