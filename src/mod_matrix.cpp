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
        menu.CustomOnlyMode = true;
        menu.SetLength(3);

        menu.RenderCustomDisplayCallback = [this](U8G2* display)
        {
            int offset = (this->activeRoute+1) < 10 ? 0 : 3;
            char val[16];

            display->clearDisplay();
            display->setDrawColor(1);
            display->setCursor(56-offset, 12);
            display->print("[");
            display->setCursor(63-offset, 12);
            display->print(this->activeRoute+1);
            display->setCursor(70+offset, 12);
            display->print("]");

            display->setDrawColor(1);
            display->drawFrame(0, 31, 63, 17);
            display->drawFrame(0, 47, 63, 17);
            display->drawFrame(65, 47, 63, 17);

            for (int k = 0; k < 4; k++)
            {
                YieldAudio();
                
                int item = 999;
                if (k == 0) item = 0;
                if (k == 1) item = 3;
                if (k == 2) item = 1;
                if (k == 3) item = 2;
                if (item >= 3)
                    continue;

                int x = k % 2;
                int y = k / 2;

                menu.Formatters[item](item, menu.Values[item], menu.GetScaledValue(item), val);
                int w = display->getStrWidth(val);
                int xPos = x == 0 ? 32 - w/2 : 96 - w/2;
                display->setCursor(xPos, y == 0 ? 44 : 60);
                display->print(val);

                YieldAudio();
                display->drawBox(x == 0 ? 1 : 66, y == 0 ? 32 : 48, menu.Values[item] * 61, 2);
            }
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
            int slot = sv;
            char name[16];
            strcpy(name, "-");
            voice.Gen->GetModSlotName(slot, name);
            strcpy(dest, name);
        };

        menu.Formatters[2] = [](int idx, float val, int sv, char* dest) { sprintf(dest, "%.0f%%", (val-0.5) * 200.0f); };

        menu.Steps[0] = 13;
        menu.Steps[1] = 4; //voice.Gen->GetModSlots();
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
            
            menu->SetScaledValue(0, (int)this->Routes[activeRoute].Source);
            menu->SetScaledValue(1, this->Routes[activeRoute].Slot);
            menu->Values[2] = this->Routes[activeRoute].Amount;

            auto newSv = menu->GetScaledValue(0);
            LogInfof("new scaledValue: %d", newSv);
        };

        menu.HandlePotCallback = [](Menu* menu, int idx, float value)
        {
            menu->SetValue(idx, value);
        };

        menu.ValueChangedCallback = [this](int idx, float val, int sv)
        {
            LogInfof("Valuechange: %.3f, %d", val, sv);
            if (idx == 0) this->Routes[activeRoute].Source = (ModSource)sv;
            if (idx == 1) this->Routes[activeRoute].Slot = sv;
            if (idx == 2) this->Routes[activeRoute].Amount = val;
        };
    }
}