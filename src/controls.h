#include "Arduino.h"
#include "pins.h"

struct PotUpdate
{
    float Value;
    bool IsNew;

    PotUpdate(float val, bool isNew)
    {
        Value = val;
        IsNew = isNew;
    }
};

// Note: Ideal update frequency for this is about 1Khz
class ControlManager
{
    int ButtonCounter[4] = {0};
    bool ButtonState[4] = {false};
    float PotState[4] = {0.0};
    
    // used to smooth pot behaviour
    float PotMomentum[4] = {0.0};
    float PotOutputValue[4] = {0.0};

    // clip the top and bottom range to ensure the pot can reach min and max reliably
    const int PotDeadSpace = 4;
    const float PotScaler = 1024.0 / (1024.0 - 2*PotDeadSpace);
    const int PotHysteresis = 4;

public:
    bool EnableFilter = true;

    inline void UpdatePotState(int pot)   
    {
        float prevVal = PotState[pot];

        int pin = 0;
        if (pot == 0) pin = PIN_POT0;
        if (pot == 1) pin = PIN_POT1;
        if (pot == 2) pin = PIN_POT2;
        if (pot == 3) pin = PIN_POT3;
        if (EnableFilter)
            PotState[pot] = PotState[pot] * 0.9 + analogRead(pin) * 0.1;
        else
            PotState[pot] = analogRead(pin);

        float delta = PotState[pot] - prevVal;
        PotMomentum[pot] = PotMomentum[pot] * 0.99 + delta;
    }

    inline float GetPotMomentum(int pot)
    {
        if (pot < 0 || pot > 3)
            return 0;

        return PotMomentum[pot];
    }

private:
    inline float ScalePot(float p)
    {
        p = (p - PotDeadSpace) * PotScaler;
        p = p < 0 ? 0 : p;
        p = p > 1023 ? 1023 : p;
        return p;
    }

public:
    inline PotUpdate GetPot(int pot)
    {
        if (pot < 0 || pot > 3)
            return PotUpdate(0, false);

        float currentOutput = PotOutputValue[pot];
        float p = ScalePot(PotState[pot]);
        float delta = fabsf(p - currentOutput);

        bool newValue = delta >= 1;
        //LogInfof("potState: %.3f p: %.3f old: %.3f newValue: %d", PotState[pot], p, PotOutputValue[pot], newValue ? 1 : 0)
        bool snapToMax = p >= 1022 && currentOutput != 1023;
        bool snapToMin = p <= 1 && currentOutput != 0;

        // new value is sufficiently different from old one, or momentum is high - emit new value
        if (newValue && ((delta > PotHysteresis) || (fabsf(PotMomentum[pot]) > 1.5)) || snapToMax || snapToMin)
        {
            //LogInfof("Potstate: %.3f p: %.3f", PotState[pot], p);
            PotOutputValue[pot] = p;
            return PotUpdate(p, true);
        }
        else
        {
            return PotUpdate(PotOutputValue[pot], false);
        }
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
