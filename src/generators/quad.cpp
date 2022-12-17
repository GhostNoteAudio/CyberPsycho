#include "quad.h"
#include "generatorRegistry.h"
#include "menus.h"

namespace Cyber
{
    Quad::Quad()
    {
        int blankIdx = generatorRegistry.GetSlotGenIndexById("Slot-GNA-Blank");
        Slots[0] = generatorRegistry.CreateSlotGenInstance(blankIdx);
        Slots[1] = generatorRegistry.CreateSlotGenInstance(blankIdx);
        Slots[2] = generatorRegistry.CreateSlotGenInstance(blankIdx);
        Slots[3] = generatorRegistry.CreateSlotGenInstance(blankIdx);

        auto formatter = [this](int slot, int idx, char* dest)
        {
            Slots[slot]->GetParamDisplay(idx, dest);
        };

        for (int i = 0; i < 64; i++)
        {
            int slot = i / 16;
            int param = i % 16;
            menu.Formatters[i] = [formatter, slot, param](int idx, float value, int sv, char* dest) { formatter(slot, param, dest); };
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
            Slots[slot]->ParamUpdated(param);
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
            if (selectionModeActive && value) selectionModeActive = false;
            
            HandleSwitchDefault(menu, idx, value);
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
        sprintf(dest, "Slot %d", idx);
    }

    int Quad::ResolveSlot(int knobIdx)
    {
        return 0;
    }

    void Quad::Process(GeneratorArgs args)
    {
        SlotArgs sargs;
        sargs.Bpm = args.Bpm;
        
        for (int i = 0; i < args.Size; i++)
        {
            sargs.Gate = args.Data->Gate[0][i];
            sargs.Input = args.Data->Mod[0][i];
            sargs.Output = 0.0f;
            Slots[0]->Process(&sargs);
            args.Data->Out[0][i] = sargs.Output;

            sargs.Gate = args.Data->Gate[1][i];
            sargs.Input = args.Data->Mod[1][i];
            sargs.Output = 0.0f;
            Slots[1]->Process(&sargs);
            args.Data->Out[1][i] = sargs.Output;
        }
    }

    void Quad::RenderSlotSelectionMenu(U8G2* display)
    {
        if (!selectionModeActive) return;
        auto info = generatorRegistry.GetSlotGenInfo(selectedGen);
        display->clearDisplay();
        display->setFont(DEFAULT_FONT);
        display->setCursor(10, 30);
        display->print(info.DisplayName);
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
