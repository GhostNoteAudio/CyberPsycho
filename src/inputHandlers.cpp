#include "cyberpsycho.h"

namespace Cyber
{
    using namespace Menus;
    
    void HandleEncoderDefault(Menu* menu, int tick)
    {
        if (!menu->EditMode)
        {
            if (tick == 1)
                menu->MoveDown();
            if (tick == -1)
                menu->MoveUp();
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
            generatorSelectMenu.Values[1] = generatorRegistry.GetGenCount();
            displayManager.ActiveMenu = &generatorSelectMenu;
            displayManager.ActiveMenu->EditMode = false;
            generatorSelectMenu.EditMode = true;
            return;
        }

        // Assign new generator to active voice if changed
        if (menu == &generatorSelectMenu && value)
        {
            int selectedGen = menu->Values[0]; // This is OK, we have hacked the menu to carry an integer here
            voice.SetGenerator(selectedGen);
            displayManager.ActiveMenu = voice.Gen->GetMenu();
            displayManager.ActiveMenu->EditMode = false;
            return;
        }
    }

    void HandlePotDefault(Menu* menu, int idx, float value)
    {
        auto modSource = modalState.GetModSource();
        
        if (modSource != ModSource::Off && menu == voice.Gen->GetMenu())
        {
            int slot = voice.Gen->ResolveSlot(idx);
            voice.matrix.UpdateRoute(modSource, slot, 2*value-1);
            modalState.EnableAction = false;
            return;
        }

        if (menu->QuadMode)
        {
            if (menu->IsVisible(menu->TopItem + idx))
                menu->SetValue(menu->TopItem + idx, value);
        }
    }

    void HandleSwitchDefault(Menu* menu, int idx, bool value)
    {
        bool down = value;
        bool up = !value;
        char tabval[16];

        if (modalState.EnableAction)
        {
            if (idx < 4)
                voice.Gen->GetTab(idx, tabval);

            if (idx == 0)
            {
                if (modalState.Shift() && up)
                {
                    voice.Gen->SetTab(0);
                    controls.SetLedPage(0);
                    displayManager.ActiveMenu = voice.Gen->GetMenu();
                    displayManager.ActiveMenu->EditMode = false;
                }
            }

            if (idx == 1)
            {
                if (modalState.Shift() && down)
                {
                    displayManager.ActiveMenu = &globalMenu;
                    displayManager.ActiveMenu->EditMode = false;
                    controls.SetLedPage(-1);
                    modalState.EnableAction = false;
                }
                else if (!modalState.Shift() && up)
                {
                    if (strlen(tabval) > 0)
                    {
                        voice.Gen->SetTab(1);
                        controls.SetLedPage(1);
                        displayManager.ActiveMenu = voice.Gen->GetMenu();
                        displayManager.ActiveMenu->EditMode = false;
                    }
                }
            }

            if (idx == 2)
            {
                if (!modalState.Shift() && up && strlen(tabval) > 0)
                {
                    voice.Gen->SetTab(2);
                    controls.SetLedPage(2);
                    displayManager.ActiveMenu = voice.Gen->GetMenu();
                    displayManager.ActiveMenu->EditMode = false;
                }
            }

            if (idx == 3)
            {
                if (modalState.Shift() && down)
                {
                    Storage::SaveGlobalState();
                    modalState.EnableAction = false;
                }
                else if (!modalState.Shift() && up && strlen(tabval) > 0)
                {
                    voice.Gen->SetTab(3);
                    controls.SetLedPage(3);
                    displayManager.ActiveMenu = voice.Gen->GetMenu();
                    displayManager.ActiveMenu->EditMode = false;
                }
            }

            if (idx >= 4)
            {
                if (modalState.Shift() && down)
                    modalState.EnableAction = false;
            }
        }

        modalState.SetButtonState(idx, value);
        modalState.CheckEnableAction();
        if (!value) voice.matrix.Cleanup();
    }
}
