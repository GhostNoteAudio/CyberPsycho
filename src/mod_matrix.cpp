#include "mod_matrix.h"
#include "logging.h"
#include "voice.h"

namespace Cyber
{
    void ModMatrix::InitMenu()
    {
        /*
        ModSource Source;
        uint8_t Slot;
        float Amount;
        */
        menu.QuadMode = true;
        menu.DisableTabs = true;
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
            switch ((ModSource)sv)
            {
                case ModSource::Off: strcpy(dest, "Off"); break;
                case ModSource::Mod1: strcpy(dest, "Mod 1"); break;
                case ModSource::Mod2: strcpy(dest, "Mod 2"); break;
                case ModSource::Mod3: strcpy(dest, "Mod 3"); break;
                case ModSource::Mod4: strcpy(dest, "Mod 4"); break;
                case ModSource::Cv1: strcpy(dest, "CV 1"); break;
                case ModSource::Cv2: strcpy(dest, "CV 2"); break;
                case ModSource::Cv3: strcpy(dest, "CV 3"); break;
                case ModSource::Cv4: strcpy(dest, "CV 4"); break;
                case ModSource::Gate1: strcpy(dest, "Gate 1"); break;
                case ModSource::Gate2: strcpy(dest, "Gate 2"); break;
                case ModSource::Gate3: strcpy(dest, "Gate 3"); break;
                case ModSource::Gate4: strcpy(dest, "Gate 4"); break;
            }
        };

        menu.Formatters[1] = [this](int idx, float val, int sv, char* dest) 
        {
            int tab = sv >> 4;
            int subslot = sv & 0b1111;

            auto menu = voice.Gen->GetMenu(tab);
            if (subslot < menu->GetLength())
                strcpy(dest, menu->Captions[subslot]);
            else
                strcpy(dest, "-");
        };

        menu.Formatters[2] = [](int idx, float val, int sv, char* dest) { sprintf(dest, "%.0f%%", (val-0.5) * 200.0f); };

        menu.Steps[0] = 12;
        menu.Steps[1] = 64;
        menu.Captions[0] = "Source";
        menu.Captions[1] = "Slot";
        menu.Captions[2] = "Amount";

        menu.HandleEncoderCallback = [this](Menu* menu, int tick)
        {
            if (tick < 0 && this->activeRoute == 0)
                return;
            if (tick > 0 && this->activeRoute == ModRouteCount-1)
                return;
            this->activeRoute += tick;
            LogInfof("Active Route: %d", this->activeRoute);

            menu->Values[0] = (int)this->Routes[activeRoute].Source;
            menu->Values[1] = this->Routes[activeRoute].Slot;
            menu->Values[2] = this->Routes[activeRoute].Amount;
        };

        menu.HandlePotCallback = [](Menu* menu, int idx, float value)
        {
            menu->SetValue(idx, value);
        };

        menu.ValueChangedCallback = [this](int idx, float val, int sv)
        {
            if (idx == 0) this->Routes[activeRoute].Source = (ModSource)sv;
            if (idx == 1) this->Routes[activeRoute].Slot = sv;
            if (idx == 2) this->Routes[activeRoute].Amount = val;
        };
    }
}