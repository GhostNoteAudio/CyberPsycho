#include "quad.h"
#include "generatorRegistry.h"

namespace Cyber
{
    Quad::Quad()
    {
        Slots[0] = generatorRegistry.CreateSlotGenInstance(0);
        Slots[1] = generatorRegistry.CreateSlotGenInstance(1);

        auto formatter = [this](int slot, int idx, char* dest)
        {
            Slots[slot]->GetParamDisplay(idx, dest);
        };

        for (int i = 0; i < 32; i++)
        {
            int slot = i / 16;
            int param = i % 16;
            menu.Captions[i] = Slots[slot]->GetParamName(param);
            menu.Formatters[i] = [formatter, slot, param](int idx, float value, int sv, char* dest) { formatter(slot, param, dest); };
            menu.Values[i] = Slots[slot]->Param[param];
        }

        menu.ValueChangedCallback = [this](int idx, float value, int sv)
        {
            int slot = idx / 16;
            int param = idx % 16;
            Slots[slot]->Param[param] = value;
            Slots[slot]->ParamUpdated(param);
        };
        
        menu.QuadMode = true;
        menu.SetLength(64);

        menu.AddSectionBreak( 0 + Slots[0]->ParamCount - 1);
        menu.AddSectionBreak(16 + Slots[1]->ParamCount - 1);

        menu.AddSectionBreak(16-1);
        menu.AddSectionBreak(32-1);
        menu.AddSectionBreak(48-1);
    }

    void Quad::GetTab(int idx, char* dest)
    {
        if (idx == 0) strcpy(dest, Slots[0]->TabName);
        if (idx == 1) strcpy(dest, Slots[1]->TabName);
        if (idx == 2) strcpy(dest, "Slot3");
        if (idx == 3) strcpy(dest, "Slot4");
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
}
