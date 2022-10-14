#include "Arduino.h"
#include "pins.h"

class ControlManager
{
    int ButtonCounter[4] = {0};
    bool ButtonState[4] = {false};
    float PotState[4] = {0.0};

    // clip the top and bottom range to ensure the pot can reach min and max reliably
    const int PotDeadSpace = 4;
    const float PotScaler = 1.0 / (1023.0 - 2*PotDeadSpace);

public:
    bool EnableFilter = false;

    inline void UpdatePotState(int pot)   
    {
        int pin = 0;
        if (pot == 0) pin = PIN_POT0;
        if (pot == 1) pin = PIN_POT1;
        if (pot == 2) pin = PIN_POT2;
        if (pot == 3) pin = PIN_POT3;
        if (EnableFilter)
            PotState[pot] = PotState[pot] * 0.9 + analogRead(pin) * 0.1;
        else
            PotState[pot] = analogRead(pin);
    }

    inline float GetPot(int pot)
    {
        if (pot < 0 || pot > 3)
            return 0;

        float p = PotState[pot];
        p = (p - PotDeadSpace) * PotScaler;
        p = p < 0 ? 0 : p;
        p = p > 1 ? 1 : p;
        return p;
    }

    inline void UpdateButtonState()
    {
        int b0 = digitalRead(PIN_BTN0);
        int b1 = digitalRead(PIN_BTN1);
        int b2 = digitalRead(PIN_BTN2);
        int b3 = digitalRead(PIN_BTN3);

        int scaler = EnableFilter ? 2 : 300;
        int halfScaler = scaler >> 1;
        ButtonCounter[0] += (-halfScaler + scaler*b0);
        ButtonCounter[1] += (-halfScaler + scaler*b1);
        ButtonCounter[2] += (-halfScaler + scaler*b2);
        ButtonCounter[3] += (-halfScaler + scaler*b3);

        if (ButtonCounter[0] < 0)
            ButtonCounter[0] = 0;
        if (ButtonCounter[1] < 0)
            ButtonCounter[1] = 0;
        if (ButtonCounter[2] < 0)
            ButtonCounter[2] = 0;
        if (ButtonCounter[3] < 0)
            ButtonCounter[3] = 0;

        if (ButtonCounter[0] > 15)
            ButtonCounter[0] = 15;
        if (ButtonCounter[1] > 15)
            ButtonCounter[1] = 15;
        if (ButtonCounter[2] > 15)
            ButtonCounter[2] = 15;
        if (ButtonCounter[3] > 15)
            ButtonCounter[3] = 15;

        if (ButtonCounter[0] > 12)
            ButtonState[0] = true;
        if (ButtonCounter[1] > 12)
            ButtonState[1] = true;
        if (ButtonCounter[2] > 12)
            ButtonState[2] = true;
        if (ButtonCounter[3] > 12)
            ButtonState[3] = true;

        if (ButtonCounter[0] < 3)
            ButtonState[0] = false;
        if (ButtonCounter[1] < 3)
            ButtonState[1] = false;
        if (ButtonCounter[2] < 3)
            ButtonState[2] = false;
        if (ButtonCounter[3] < 3)
            ButtonState[3] = false;
    }

    inline bool GetButton(int button)
    {
        if (button < 0 || button > 3)
            return false;

        return ButtonState[button];
    }
};
