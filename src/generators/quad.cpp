#include "quad.h"
#include "generatorRegistry.h"
#include "menus.h"
#include "modal_state.h"

namespace Cyber
{
    Quad::Quad()
    {
        int blankIdx = generatorRegistry.GetSlotGenIndexById("S-GNA-Blank");
        Slots[0] = generatorRegistry.CreateSlotGenInstance(blankIdx);
        Slots[1] = generatorRegistry.CreateSlotGenInstance(blankIdx);
        Slots[2] = generatorRegistry.CreateSlotGenInstance(blankIdx);
        Slots[3] = generatorRegistry.CreateSlotGenInstance(blankIdx);

        auto formatter = [this](int slot, int idx, float value, char* dest)
        {
            Slots[slot]->GetParamDisplay(idx, value, dest);
        };

        for (int i = 0; i < 64; i++)
        {
            int slot = i / 16;
            int param = i % 16;
            menu.Formatters[i] = [formatter, slot, param](int idx, float value, int sv, char* dest) { formatter(slot, param, value, dest); };
        }

        menu.QuadMode = true;
        menu.SetLength(64);
        UpdateMenuSections();
        UpdateCaptionsValues();

        menu.ValueChangedCallback = [this](int idx, float value, int sv)
        {
            int slot = idx / 16;
            int param = idx % 16;
            Slots[slot]->Param[param] = value;
            Slots[slot]->ParamDirty[param] = true;
        };
        
        menu.RenderCustomDisplayCallback = [this](U8G2* display)
        {
            RenderSlotSelectionMenu(display);
        };

        menu.HandleEncoderCallback = [this](Menu* menu, int tick)
        {
            if (!selectionModeActive)
            {
                HandleEncoderDefault(menu, tick);
                return;
            }

            int newSelectedGen = selectedGen + tick;
            if (newSelectedGen >= generatorRegistry.GetSlotGenCount())
                newSelectedGen = generatorRegistry.GetSlotGenCount() - 1;
            if (newSelectedGen < 0)
                newSelectedGen = 0;

            selectedGen = newSelectedGen;
        };

        menu.HandleEncoderSwitchCallback = [this](Menu* menu, bool value)
        {
            if (!modalState.Shift() && !selectionModeActive)
            {
                HandleEncoderSwitchDefault(menu, value);
                return;
            }

            if (!value) return;

            if (!selectionModeActive)
            {
                selectionModeActive = true;
                menu->DisableTabs = true;
                modalState.EnableAction = false;
                selectedGen = Slots[ActiveTab]->GenIndex;
            }
            else
            {
                selectionModeActive = false;
                menu->DisableTabs = false;
                if (selectedGen != Slots[ActiveTab]->GenIndex)
                {
                    LogInfof("Load Gen %d into slot %d !", selectedGen, ActiveTab);
                    SetSlotGen(ActiveTab, selectedGen);
                }
            }
        };

        menu.HandleSwitchCallback = [this](Menu* menu, int idx, bool value)
        {
            if (selectionModeActive && value)
            {
                selectionModeActive = false;
                menu->DisableTabs = false;
            }
            
            HandleSwitchDefault(menu, idx, value);
        };

        menu.HandlePotCallback = [this](Menu* menu, int idx, float value)
        {
            if (!selectionModeActive)
            {
                HandlePotDefault(menu, idx, value);
                return;
            }

            if (idx == 0)
                Inputs[ActiveTab] = (Input)(value * 2.99f);
            else if (idx == 1)
                GainInDb[ActiveTab] = value;
            else if (idx == 2)
                GainOutDb[ActiveTab] = value;
        };
    }

    void Quad::GetTab(int idx, char* dest)
    {
        if (idx == 0) strcpy(dest, Slots[0]->TabName);
        if (idx == 1) strcpy(dest, Slots[1]->TabName);
        if (idx == 2) strcpy(dest, Slots[2]->TabName);
        if (idx == 3) strcpy(dest, Slots[3]->TabName);
    }

    void Quad::SetTab(int tab)
    {
        menu.SetSelectedItem(tab * 16);
        ActiveTab = tab;
    }

    Menu* Quad::GetMenu()
    {
        return &menu;
    }

    int Quad::GetModSlots()
    {
        return 64;
    }

    void Quad::GetModSlotName(int idx, char* dest)
    {
        strcpy(dest, menu.Captions[idx]);
    }

    int Quad::ResolveSlot(int knobIdx)
    {
        int page = menu.GetPage();
        int slot = ActiveTab;
        int idx = slot*16 + page*4 + knobIdx;
        LogInfof("Resolve slot as %d", idx);
        return idx;
    }

    void Quad::Process(GeneratorArgs args)
    {
        SlotArgs sargs;
        sargs.Bpm = args.Bpm;
        float zeros[BUFFER_SIZE] = {0};

        for (int n = 0; n < 4; n++)
        {
            for (int i = 0; i < Slots[n]->ParamCount; i++)
            {
                float oldVal = Slots[n]->Param[i];
                float newVal = Utils::Clamp(menu.Values[n*16 + i] + args.GetModulationSlow(n*16 + i));
                Slots[n]->Param[i] = newVal;
                Slots[n]->ParamDirty[i] |= oldVal != newVal;
            }
            Slots[n]->ParamUpdated();
        }

        for (int n = 0; n < 4; n++)
        {
            float* inArr;
            if (Inputs[n] == INPUT_OFF)
                inArr = zeros;
            else if (Inputs[n] == INPUT_EXT)
                inArr = args.Data->Mod[n];
            else if (Inputs[n] == INPUT_PREV && n == 0)
                inArr = zeros;
            else
                inArr = args.Data->Out[n-1];

            bool* gateArr = args.Data->Gate[n];
            float* cvArr = args.Data->Cv[n];
            float inGain = Utils::DB2Gainf(-12 + 24 * GainInDb[n]);
            float outGain = Utils::DB2Gainf(-12 + 24 * GainOutDb[n]);
            float* outArr = args.Data->Out[n];

            for (int i = 0; i < args.Size; i++)
            {
                sargs.Gate = gateArr[i];
                sargs.Cv = cvArr[i];
                sargs.Input = inArr[i] * inGain;
                sargs.Output = 0.0f;
                Slots[n]->Process(&sargs);
                outArr[i] = sargs.Output * outGain;
            }
        }
    }

    void Quad::SaveState(uint8_t* buffer, int maxLength)
    {
        strcpy((char*)&buffer[0], generatorRegistry.GetSlotGenInfo(Slots[0]->GenIndex).GeneratorId);
        strcpy((char*)&buffer[16], generatorRegistry.GetSlotGenInfo(Slots[1]->GenIndex).GeneratorId);
        strcpy((char*)&buffer[32], generatorRegistry.GetSlotGenInfo(Slots[2]->GenIndex).GeneratorId);
        strcpy((char*)&buffer[48], generatorRegistry.GetSlotGenInfo(Slots[3]->GenIndex).GeneratorId);
        
        buffer[64] = (uint8_t)Inputs[0];
        buffer[65] = (uint8_t)Inputs[1];
        buffer[66] = (uint8_t)Inputs[2];
        buffer[67] = (uint8_t)Inputs[3];

        float* fptr = (float*)(buffer + 68);
        fptr[0] = GainInDb[0];
        fptr[1] = GainInDb[1];
        fptr[2] = GainInDb[2];
        fptr[3] = GainInDb[3];

        fptr[4] = GainOutDb[0];
        fptr[5] = GainOutDb[1];
        fptr[6] = GainOutDb[2];
        fptr[7] = GainOutDb[3];
    }

    void Quad::LoadState(uint8_t* buffer, int length)
    {
        const char* slotGenIdStr;
        int slotGenIdx;

        for (int i = 0; i < 4; i++)
        {
            LogInfof("Loading slot generator into slot %d", i);
            slotGenIdStr = (char*)&buffer[i*16];
            slotGenIdx = generatorRegistry.GetSlotGenIndexById(slotGenIdStr);
            if (slotGenIdx == -1)
            {
                LogInfof("Cannot find Slot Generator with Id: %s", slotGenIdStr);
            }
            else
            {
                LogInfof("SlotGenerator Id: %s, index: %d", slotGenIdStr, slotGenIdx);
                SetSlotGen(i, slotGenIdx);
            }
        }

        Inputs[0] = (Input)buffer[64];
        Inputs[1] = (Input)buffer[65];
        Inputs[2] = (Input)buffer[66];
        Inputs[3] = (Input)buffer[67];

        float* fptr = (float*)(buffer + 68);
        GainInDb[0] = fptr[0];
        GainInDb[1] = fptr[1];
        GainInDb[2] = fptr[2];
        GainInDb[3] = fptr[3];

        GainOutDb[0] = fptr[4];
        GainOutDb[1] = fptr[5];
        GainOutDb[2] = fptr[6];
        GainOutDb[3] = fptr[7];
    }

    void Quad::RenderSlotSelectionMenu(U8G2* display)
    {
        if (!selectionModeActive) return;
        char val[16];
        auto info = generatorRegistry.GetSlotGenInfo(selectedGen);
        int w = display->getStrWidth(info.DisplayName);
        display->clearDisplay();
        display->setFont(DEFAULT_FONT);
        display->setDrawColor(1);
        display->setCursor(64 - w/2, 18);
        display->print(info.DisplayName);

        sprintf(val, "(%d/%d)", selectedGen+1, generatorRegistry.GetSlotGenCount());
        w = display->getStrWidth(val);
        display->setCursor(96 - w/2, 43);
        display->print(val);

        display->drawFrame(0, 31, 63, 17);
        display->drawFrame(0, 47, 63, 17);
        display->drawFrame(65, 47, 63, 17);

        auto inp = (int)Inputs[ActiveTab];
        const char* inVal = inp == 0 ? "Off" : inp == 1 ? "Ext" : "Previous";
        w = display->getStrWidth(inVal);
        display->setCursor(32 - w/2, 44);
        display->print(inVal);

        sprintf(val, "In: %.1f", (-12 + 24 * GainInDb[ActiveTab]));
        w = display->getStrWidth(val);
        display->setCursor(32 - w/2, 60);
        display->print(val);

        sprintf(val, "Out: %.1f", (-12 + 24 * GainOutDb[ActiveTab]));
        w = display->getStrWidth(val);
        display->setCursor(96 - w/2, 60);
        display->print(val);

        display->drawBox(1, 32, ((int)Inputs[ActiveTab]) * 0.5f * 61, 2);
        display->drawBox(1, 48, GainInDb[ActiveTab] * 61, 2);
        display->drawBox(66, 48, GainOutDb[ActiveTab] * 61, 2);
    }

    void Quad::UpdateMenuSections()
    {
        menu.ClearAllSectionBreaks();
        if (Slots[0]->ParamCount == 0) menu.AddSectionBreak(0);
        if (Slots[1]->ParamCount == 0) menu.AddSectionBreak(16);
        if (Slots[2]->ParamCount == 0) menu.AddSectionBreak(32);
        if (Slots[3]->ParamCount == 0) menu.AddSectionBreak(48);
        menu.AddSectionBreak( 0 + Slots[0]->ParamCount - 1);
        menu.AddSectionBreak(16 + Slots[1]->ParamCount - 1);
        menu.AddSectionBreak(32 + Slots[2]->ParamCount - 1);
        menu.AddSectionBreak(48 + Slots[3]->ParamCount - 1);
        menu.AddSectionBreak(16-1);
        menu.AddSectionBreak(32-1);
        menu.AddSectionBreak(48-1);
    }

    void Quad::UpdateCaptionsValues()
    {
        for (int i = 0; i < 64; i++)
        {
            int slot = i / 16;
            int param = i % 16;
            menu.Captions[i] = Slots[slot]->GetParamName(param);
            menu.Values[i] = Slots[slot]->Param[param];
        }
    }

    void Quad::SetSlotGen(int slot, int genId)
    {
        generatorRegistry.DeleteSlotGenInstance(Slots[slot]);
        Slots[slot] = generatorRegistry.CreateSlotGenInstance(genId);
        UpdateMenuSections();
        UpdateCaptionsValues();
    }
}
