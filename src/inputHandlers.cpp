#include "cyberpsycho.h"

namespace Cyber
{
    using namespace Menus;
    
    void HandleEncoderDefault(Menu* menu, int tick)
    {
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
        if (voice.Gen->GetMenu() == menu && value)
        {
            generatorSelectMenu.Values[0] = voice.Gen->GenIndex;
            generatorSelectMenu.Values[1] = generatorRegistry.Count;
            displayManager.ActiveMenu = &generatorSelectMenu;
            generatorSelectMenu.EditMode = true;
            return;
        }

        // Assign new generator to active voice if changed
        if (menu == &generatorSelectMenu && value)
        {
            int selectedGen = menu->Values[0]; // This is OK, we have hacked the menu to carry an integer here
            voice.SetGenerator(selectedGen);
            displayManager.ActiveMenu = voice.Gen->GetMenu();
            return;
        }
    }

    void HandlePotDefault(Menu* menu, int idx, float value)
    {
        if (menu->QuadMode)
        {
            menu->SetValue(menu->TopItem + idx, value);
        }
    }

    void HandleSwitchDefault(Menu* menu, int idx, bool value)
    {
        bool down = value;
        bool up = !value;

        if (modalState.EnableAction)
        {
            if (idx == 0)
            {
                if (modalState.Shift() && up)
                {
                    voice.Gen->SetTab(0);
                    displayManager.ActiveMenu = voice.Gen->GetMenu();
                }
            }

            if (idx == 1)
            {
                if (modalState.Shift() && down)
                {
                    displayManager.ActiveMenu = &globalMenu;
                    modalState.EnableAction = false;
                }
                else if (!modalState.Shift() && up)
                {
                    if (strlen(voice.Gen->GetTabs()[idx]) > 0)
                    {
                        voice.Gen->SetTab(1);
                        displayManager.ActiveMenu = voice.Gen->GetMenu();
                    }
                }
            }

            if (idx == 2 || idx == 3)
            {
                if (!modalState.Shift() && up && strlen(voice.Gen->GetTabs()[idx]) > 0)
                {
                    voice.Gen->SetTab(idx);
                    displayManager.ActiveMenu = voice.Gen->GetMenu();
                }
            }
        }

        modalState.ButtonState[idx] = value;
        modalState.CheckEnableAction();
    }
}
