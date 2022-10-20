#pragma once
#include "menu.h"

namespace Cyber
{    
    class ModalState
    {
    public:
        bool Shift;
    };

    extern ModalState modalState;

    namespace Menus
    {
        extern Menu* ActiveMenu;

        extern Menu initMenu;
        extern Menu globalMenu;
        extern Menu scopeMenu;
        extern Menu calibrateMenu;
        extern Menu voiceMenu;
        extern Menu pitchTrigMenu;

        void Init();
    }
}
