#include "cyberpsycho.h"

namespace Cyber
{
    using namespace Menus;
    
    void HandleEncoderDefault(Menu* menu, int tick)
    {
        if (!menu->EditMode)
        {
            if (menu == Voices[ActiveVoice].generator->GetMenu())
            {
                if (menu->TopItem == 0 && tick == -1)
                {
                    ActiveMenu = &generatorSelectMenu;
                    return;
                }
            }
            if (menu == &generatorSelectMenu && tick == 1)
            {
                ActiveMenu = Voices[ActiveVoice].generator->GetMenu();
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
    }

    void HandlePotDefault(Menu* menu, int idx, float value)
    {
        if (menu->QuadMode)
        {
            menu->SetValue(menu->TopItem + idx, value * 100);
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
                    ActiveMenu = Voices[ActiveVoice].generator->GetMenu();
                }
                else
                {
                    // noop
                }
                modalState.Shift = false;
            }
        }
        else if (idx == 1 && value && modalState.Shift)
        {
            ActiveMenu = &globalMenu;
            modalState.Shift = false;
        }
        else if (idx == 2 && value && modalState.Shift)
        {
            ActiveMenu = &voiceMenu;
            modalState.Shift = false;
        }
        else if (idx == 3 && value && modalState.Shift)
        {
            ActiveMenu = &pitchTrigMenu;
            modalState.Shift = false;
        }
    }
}
