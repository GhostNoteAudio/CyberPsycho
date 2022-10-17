#pragma once
#include "Arduino.h"
#include "pins.h"
#include "logging.h"
#include "utils.h"

namespace Cyber
{
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

    const int POT_FIR_SIZE = 60;

    // Note: Ideal update frequency for this is about 1Khz
    class ControlManager
    {
        int ButtonCounter[4] = {0};
        bool ButtonState[4] = {false};
        float PotState[4][POT_FIR_SIZE] = {{0.0}};
        int potStateIdx[4] = {0};
        
        // used to smooth pot behaviour
        float PotExcursionPoint[4] = {0.0};
        float PotMomentum[4] = {0.0};
        float PotOutputValue[4] = {0.0};

        // clip the top and bottom range to ensure the pot can reach min and max reliably
        const int PotDeadSpace = 4;
        const float PotScaler = 1024.0 / (1024.0 - 2*PotDeadSpace);
        const float Pot10BitScale = 1.0 / 1023.0;
        const int PotHysteresis = 10;
        const int PotExcursionJump = 4;

    public:
        inline void UpdatePotState(int pot)   
        {
            potStateIdx[pot] = (potStateIdx[pot] + 1) % POT_FIR_SIZE;
            int idxWrite = potStateIdx[pot];

            int pin = 0;
            if (pot == 0) pin = PIN_POT0;
            if (pot == 1) pin = PIN_POT1;
            if (pot == 2) pin = PIN_POT2;
            if (pot == 3) pin = PIN_POT3;
            float newVal = analogRead(pin);
            PotState[pot][idxWrite] = newVal;
            float meanVal = Utils::Mean(PotState[pot], POT_FIR_SIZE);

            float delta = 0;
            if (fabsf(meanVal - PotExcursionPoint[pot]) > PotExcursionJump)
            {
                PotExcursionPoint[pot] = meanVal;
                delta = 2 * PotExcursionJump;
            }

            PotMomentum[pot] = PotMomentum[pot] * 0.99 + delta;
            PotMomentum[pot] = PotMomentum[pot] > 200 ? 200 : PotMomentum[pot];          
        }

        inline float GetPotMomentum(int pot)
        {
            if (pot < 0 || pot > 3)
                return 0;

            return PotMomentum[pot];
        }

        inline float GetPotExcursionPoint(int pot)
        {
            if (pot < 0 || pot > 3)
                return 0;

            return PotExcursionPoint[pot];
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
        inline float GetPotRaw(int pot)
        {
            float potVal = Utils::Mean(PotState[pot], POT_FIR_SIZE);
            return ScalePot(potVal) * Pot10BitScale;
        }

        inline PotUpdate GetPot(int pot)
        {
            if (pot < 0 || pot > 3)
                return PotUpdate(0, false);

            float currentOutput = PotOutputValue[pot];
            float pv = Utils::Mean(PotState[pot], POT_FIR_SIZE);
            float p = ScalePot(pv);
            
            // Talk about over-engineered :)
            float delta = fabsf(p - currentOutput);
            bool beyondHyst = delta > PotHysteresis;
            bool isNew = delta > 0.001;
            bool atLeast1Different = delta >= 1;
            bool highMomentum = fabsf(PotMomentum[pot]) > 1.0;
            bool maxBoundary = p == 1023;
            bool minBoundary = p == 0;

            if ((isNew && beyondHyst) || (isNew && highMomentum && atLeast1Different) || (isNew && (maxBoundary || minBoundary)))
            {
                PotOutputValue[pot] = p;
                return PotUpdate(p * Pot10BitScale, true);
            }
            else
            {
                return PotUpdate(PotOutputValue[pot] * Pot10BitScale, false);
            }
        }

        inline void UpdateButtonState()
        {
            int b0 = digitalRead(PIN_BTN0);
            int b1 = digitalRead(PIN_BTN1);
            int b2 = digitalRead(PIN_BTN2);
            int b3 = digitalRead(PIN_BTN3);

            int scaler = 8;
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

    extern ControlManager controls;
}
