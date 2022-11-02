#include "cyberpsycho.h"

namespace Cyber
{
    using namespace Menus;
    
    void HandleEncoderDefault(Menu* menu, int tick)
    {
        auto voice = Voices::GetActiveVoice();
        if (menu == voice->GetActiveInsert()->GetMenu())
        {
            if (tick > 0) voice->ActiveInsert++;
            if (tick < 0) voice->ActiveInsert--;
            if (voice->ActiveInsert > 3)
                voice->ActiveInsert = 3;
            if (voice->ActiveInsert < 0)
                voice->ActiveInsert = 0;
            ActiveMenu = voice->GetActiveInsert()->GetMenu();
            return;
        }

        if (!menu->EditMode)
        {
            if (tick == 1 && !menu->QuadMode)
                menu->MoveDown();
            if (tick == 1 && menu->QuadMode)
                menu->MoveDownPage();
            if (tick == -1 && !menu->QuadMode)
                menu->MoveUp();
            if (tick == -1 && menu->QuadMode)
                menu->MoveUpPage();
        }
        else if (menu->EditMode && !menu->QuadMode)
        {
            menu->TickValue(menu->SelectedItem, tick);
        }
    }

    void HandleEncoderSwitchDefault(Menu* menu, bool value)
    {
        if (value && !menu->QuadMode)
            menu->EditMode = !menu->EditMode;

        // Go from generator edit menu to generator selection menu
        if (menu == Voices::GetActiveGen()->GetMenu() && value)
        {
            generatorSelectMenu.Values[0] = Voices::GetActiveGen()->GenIndex;
            generatorSelectMenu.Max[0] = generatorRegistry.Count-1;
            generatorSelectMenu.Values[1] = 0; // Show inserts only?
            generatorSelectMenu.Values[2] = -1; // Selected insert effect.
            ActiveMenu = &generatorSelectMenu;
            generatorSelectMenu.EditMode = true;
            return;
        }

        auto getFxMenuIndex = [](Menu* m)
        {
            auto v = Voices::GetActiveVoice();
            if (v->Inserts[v->ActiveInsert]->GetMenu() == m)
                return v->ActiveInsert;
            return -1;
        };

        int fxIdx = getFxMenuIndex(menu);

        // Go from insert FX edit menu to generator selection menu (Insert only)
        if (fxIdx != -1 && value)
        {
            generatorSelectMenu.Values[0] = Voices::GetActiveVoice()->GetActiveInsert()->GenIndex;
            generatorSelectMenu.Max[0] = generatorRegistry.Count-1;
            generatorSelectMenu.Values[1] = 1; // Show inserts only?
            generatorSelectMenu.Values[2] = fxIdx; // Selected insert effect.
            ActiveMenu = &generatorSelectMenu;
            generatorSelectMenu.EditMode = true;
            return;
        }

        // Assign new generator to active voice if changed
        if (menu == &generatorSelectMenu && value)
        {
            int selectedGen = menu->Values[0];
            auto voice = Voices::GetActiveVoice();
            int insertFxSlot = menu->Values[2];
            if (insertFxSlot == -1) // selecting main generator
            {
                if (voice->Gen->GenIndex != selectedGen)
                {
                    generatorRegistry.DeleteInstance(voice->Gen);
                    voice->Gen = generatorRegistry.CreateInstance(selectedGen);
                }
                ActiveMenu = voice->Gen->GetMenu();
            }
            else // Selecting insert FX
            {
                auto fxGen = voice->Inserts[insertFxSlot];
                if (fxGen->GenIndex != selectedGen)
                {
                    generatorRegistry.DeleteInstance(fxGen);
                    voice->Inserts[insertFxSlot] = generatorRegistry.CreateInstance(selectedGen);
                }
                ActiveMenu = voice->Inserts[insertFxSlot]->GetMenu();
            }
        }
    }

    void HandlePotDefault(Menu* menu, int idx, float value)
    {
        if (menu->QuadMode)
        {
            menu->SetValueF(menu->TopItem + idx, value);
        }
    }

    void HandleSwitchDefault(Menu* menu, int idx, bool value)
    {
        LogInfof("Handling switch %d, value %d", idx, (int)value);
        
        if (idx == 0)
        {
            if (value)
            {
                modalState.Shift = true;
            }
            else
            {
                if (modalState.Shift)
                {
                    ActiveMenu = Voices::GetActiveGen()->GetMenu();
                }
                else
                {
                    // noop
                }
                modalState.Shift = false;
            }
        }
        else if (idx == 1 && value)
        {
            if (modalState.Shift)
            {
                ActiveMenu = &globalMenu;
                modalState.Shift = false;
            }
            else
            {
                ActiveMenu = Voices::GetActiveVoice()->GetActiveInsert()->GetMenu();
            }
        }
        else if (idx == 2 && value)
        {
            if (modalState.Shift)
            {
                ActiveMenu = &voiceMenu;
                modalState.Shift = false;
            }
            else
            {
                
            }
        }
        else if (idx == 3 && value)
        {
            if (modalState.Shift)
            {
                ActiveMenu = &pitchTrigMenu;
                modalState.Shift = false;
            }
            else
            {
                
            }
        }
    }
}
