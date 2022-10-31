#include "cyberpsycho.h"

namespace Cyber
{
    using namespace Menus;
    
    void HandleEncoderDefault(Menu* menu, int tick)
    {
        if (!menu->EditMode)
        {
            // Go from generator edit menu to generator selection menu
            if (menu == Voices::GetActiveGen()->GetMenu())
            {
                if (menu->TopItem == 0 && tick == -1)
                {
                    generatorSelectMenu.Values[0] = Voices::GetActiveVoice()->GenIndex;
                    ActiveMenu = &generatorSelectMenu;
                    return;
                }
            }
            // Go from generator selection menu to generator edit menu
            if (menu == &generatorSelectMenu && tick == 1)
            {
                ActiveMenu = Voices::GetActiveGen()->GetMenu();
                return;
            }

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

        // Assign new generator to active voice if changed
        if (menu == &generatorSelectMenu && value && !menu->EditMode)
        {
            int selectedGen = menu->Values[0];
            auto voice = Voices::GetActiveVoice();
            if (voice->GenIndex != selectedGen)
            {
                delete voice->Gen;
                voice->Gen = generatorRegistry.CreateInstance(selectedGen);
                voice->GenIndex = selectedGen;
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
                ActiveMenu = Voices::GetActiveVoice()->mmf.GetMenu();
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
                ActiveMenu = Voices::GetActiveVoice()->redux.GetMenu();
            }
        }
        else if (idx == 3 && value && modalState.Shift)
        {
            ActiveMenu = &pitchTrigMenu;
            modalState.Shift = false;
        }
    }
}
