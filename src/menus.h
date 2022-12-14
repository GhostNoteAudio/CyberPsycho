#pragma once
#include "menu.h"

namespace Cyber
{    
    class ModalState
    {
    public:
        bool ButtonState[8] = {false};
        bool EnableAction = true;

        bool Shift() { return ButtonState[0]; }
        void CheckEnableAction()
        {
            EnableAction |= !(ButtonState[0] || ButtonState[1] || ButtonState[2] || ButtonState[3] ||
                              ButtonState[4] || ButtonState[5] || ButtonState[6] || ButtonState[7]);
        }
    };

    extern ModalState modalState;

    namespace Menus
    {
        extern Menu initMenu;
        extern Menu globalMenu;
        extern Menu scopeMenu;
        extern Menu calibrateMenu;
        extern Menu pitchTrigMenu;
        extern Menu generatorSelectMenu;

        void Init();
    }
}
