#pragma once
#include "menu.h"
#include "mod_source.h"

namespace Cyber
{    
    class ModalState
    {
        bool ButtonState[8] = {false};
    public:
        
        bool EnableAction = true;

        inline bool Shift() { return ButtonState[0]; }

        inline void SetButtonState(int idx, bool value)
        {
            ButtonState[idx] = value;
        }

        inline void CheckEnableAction()
        {
            EnableAction |= !(ButtonState[0] || ButtonState[1] || ButtonState[2] || ButtonState[3] ||
                              ButtonState[4] || ButtonState[5] || ButtonState[6] || ButtonState[7]);
        }

        inline uint8_t GetMask()
        {
            return ButtonState[0] | (ButtonState[1] << 1) | (ButtonState[2] << 2) | (ButtonState[3] << 3)
                | (ButtonState[4] << 4) | (ButtonState[5] << 5) | (ButtonState[6] << 6) | (ButtonState[7] << 7);
        }

        inline bool GateTrigger(int idx)
        {
            return idx < 4 && ButtonState[idx+4] && Shift();
        }

        inline ModSource GetModSource()
        {
            auto mask = GetMask();
            switch (mask)
            {
                case 0b00000001: return ModSource::Cv1;
                case 0b00000010: return ModSource::Cv2;
                case 0b00000100: return ModSource::Cv3;
                case 0b00001000: return ModSource::Cv4;
                case 0b00010000: return ModSource::Mod1;
                case 0b00100000: return ModSource::Mod2;
                case 0b01000000: return ModSource::Mod3;
                case 0b10000000: return ModSource::Mod4;
                case 0b00010001: return ModSource::Gate1;
                case 0b00100010: return ModSource::Gate2;
                case 0b01000100: return ModSource::Gate3;
                case 0b10001000: return ModSource::Gate4;
            }
            return ModSource::Off;
        }
    };

    extern ModalState modalState;
}
