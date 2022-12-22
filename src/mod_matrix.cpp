#include "mod_matrix.h"
#include "logging.h"
#include "voice.h"
#include "display_manager.h"

namespace Cyber
{
    const char* ModNames[] = 
    {
        "Off",
        "CV 1",
        "CV 2",
        "CV 3",
        "CV 4",
        "Mod 1",
        "Mod 2",
        "Mod 3",
        "Mod 4",
        "Gate 1",
        "Gate 2",
        "Gate 3",
        "Gate 4",
    };

    void PaintModOverlay(U8G2* display)
    {
        display->setFont(DEFAULT_FONT);
        display->setDrawColor(0);
        
        display->drawBox(20, 10, 128-40, 44);
        display->setDrawColor(1);
        display->drawFrame(20, 10, 128-40, 44);

        if (voice.matrix.LastUpdatedRoute == -1)
        {
            int w = display->getStrWidth("Mod Matrix");
            display->setCursor(64 - w/2, 28);
            display->print("Mod Matrix");

            w = display->getStrWidth("is full!");
            display->setCursor(64 - w/2, 41);
            display->print("is full!");
            return;
        }

        auto route = voice.matrix.Routes[voice.matrix.LastUpdatedRoute];
        char val[16];

        if (route.Source != ModSource::Off)
        {
            int w = display->getStrWidth(ModNames[(int)route.Source]);
            display->setCursor(64 - w/2, 22);
            display->print(ModNames[(int)route.Source]);
        }

        voice.Gen->GetModSlotName(route.Slot, val);
        int w = display->getStrWidth(val);
        display->setCursor(64 - w/2, 35);
        display->print(val);

        if (route.Amount == 0)
            strcpy(val, "Off");
        else
            sprintf(val, "%.1f%%", route.Amount * 100);
        w = display->getStrWidth(val);
        display->setCursor(64 - w/2, 48);
        display->print(val);
    }

    void ModMatrix::InitMenu()
    {
        /*
        ModSource Source;
        uint8_t Slot;
        float Amount;
        */
        menu.QuadMode = true;
        menu.DisableTabs = true;
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
            strcpy(dest, ModNames[sv]);
        };

        menu.Formatters[1] = [this](int idx, float val, int sv, char* dest) 
        {
            int slot = sv;
            char name[16];
            strcpy(name, "-");
            voice.Gen->GetModSlotName(slot, name);
            strcpy(dest, name);
        };

        menu.Formatters[2] = [](int idx, float val, int sv, char* dest) 
        { 
            float v = -1.0f + 2.0f * val;
            if (fabsf(v) < 0.02) v = 0;

            if (v == 0)
                strcpy(dest, "Off");
            else
                sprintf(dest, "%.1f%%", v * 100.0f); 
        };

        menu.Steps[0] = 13;
        menu.Steps[1] = 0; // updated whenever a generator is loaded
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
            UpdateMenuDisplay();
        };

        menu.HandlePotCallback = [](Menu* menu, int idx, float value)
        {
            menu->SetValue(idx, value);
        };

        menu.ValueChangedCallback = [this](int idx, float val, int sv)
        {
            if (idx == 0) this->Routes[activeRoute].Source = (ModSource)sv;
            if (idx == 1) this->Routes[activeRoute].Slot = sv;
            if (idx == 2)
            {
                float v = -1.0f + 2.0f * val;
                if (fabsf(v) < 0.02) v = 0;
                this->Routes[activeRoute].Amount = v;
            }
        };

        UpdateMenuDisplay();
    }

    void ModMatrix::UpdateMenuDisplay()
    {
        menu.SetScaledValue(0, (int)this->Routes[activeRoute].Source);
        menu.SetScaledValue(1, this->Routes[activeRoute].Slot);
        menu.SetValue(2, (1+this->Routes[activeRoute].Amount)*0.5);
    }

    void ModMatrix::UpdateRoute(ModSource source, int slot, float value)
    {
        if (fabsf(value) < 0.02) value = 0;

        auto findExisting = [this](ModSource source, int slot)
        {
            for (int i = 0; i < ModRouteCount; i++)
            {
                if (Routes[i].Source == source && Routes[i].Slot == slot)
                    return i;
            }
            return -1;
        };

        auto findAvailable = [this]()
        {
            for (int i = 0; i < ModRouteCount; i++)
            {
                if (Routes[i].Source == ModSource::Off || Routes[i].Amount == 0)
                    return i;
            }
            return -1;
        };

        int routeIdx = findExisting(source, slot);
        if (routeIdx == -1) routeIdx = findAvailable();

        if (routeIdx != -1)
        {
            Routes[routeIdx].Source = source;
            Routes[routeIdx].Slot = slot;
            Routes[routeIdx].Amount = value;
        }

        LastUpdatedRoute = routeIdx;
        LogInfof("UpdateRoute, slot: %d, value: %.3f, routeId: %d", slot, value, routeIdx);
        displayManager.SetOverlay(PaintModOverlay, 1000);
        UpdateMenuDisplay();
    }

    float ModMatrix::GetModulationSlow(uint8_t slot)
    {
        OutputValue = 0.0f;
        for (int i = 0; i < ModRouteCount; i++)
        {
            if (Routes[i].Slot == slot)
                AddRouteSlow(i);
        }
        
        return OutputValue;
    }

    float* ModMatrix::GetModulationFast(uint8_t slot)
    {
        Utils::ZeroBuffer(OutputBuffer, BUFFER_SIZE);
        for (int i = 0; i < ModRouteCount; i++)
        {
            if (Routes[i].Slot == slot)
                AddRouteFast(i);
        }
        
        return OutputBuffer;
    }

    void ModMatrix::Cleanup()
    {
        for (int i = 0; i < ModRouteCount; i++)
        {
            if (Routes[i].Amount == 0)
                Routes[i].Source = ModSource::Off;
        }
        UpdateMenuDisplay();
    }

    void ModMatrix::Reset()
    {
        for (int i = 0; i < ModRouteCount; i++)
        {
            Routes[i].Amount = 0;
            Routes[i].Source = ModSource::Off;
            Routes[i].Slot = 0;
        }
        UpdateMenuDisplay();
    }

    float* ModMatrix::GetSourceBuffer(ModSource source)
    {
        switch (source)
        {
            case ModSource::Off: return nullptr;
            case ModSource::Mod1: return fpData->Mod[0];
            case ModSource::Mod2: return fpData->Mod[1];
            case ModSource::Mod3: return fpData->Mod[2];
            case ModSource::Mod4: return fpData->Mod[3];
            case ModSource::Cv1: return fpData->Cv[0];
            case ModSource::Cv2: return fpData->Cv[1];
            case ModSource::Cv3: return fpData->Cv[2];
            case ModSource::Cv4: return fpData->Cv[3];
            case ModSource::Gate1: return fpData->GateFloat[0];
            case ModSource::Gate2: return fpData->GateFloat[1];
            case ModSource::Gate3: return fpData->GateFloat[2];
            case ModSource::Gate4: return fpData->GateFloat[3];
        }
        return nullptr;
    }

    void ModMatrix::AddRouteSlow(int idx)
    {
        if (Routes[idx].Source != ModSource::Off)
        {
            auto buf = GetSourceBuffer(Routes[idx].Source);
            OutputValue += buf[BUFFER_SIZE-1] * Routes[idx].Amount; // we take the LAST/most recent value from the buffer
        }
    }

    void ModMatrix::AddRouteFast(int idx)
    {
        if (Routes[idx].Source != ModSource::Off)
        {
            auto buf = GetSourceBuffer(Routes[idx].Source);
            Utils::Mix(OutputBuffer, buf, Routes[idx].Amount, BUFFER_SIZE);
        }
    }
}
