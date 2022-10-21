#pragma once
#include "Arduino.h"
#include "pins.h"
#include "logging.h"
#include "utils.h"

namespace Cyber
{
    template<typename T>
    struct ControlUpdate
    {
        T Value;
        bool IsNew;

        ControlUpdate(T val, bool isNew)
        {
            Value = val;
            IsNew = isNew;
        }
    };

    typedef ControlUpdate<float> PotUpdate;
    typedef ControlUpdate<bool> ButtonUpdate;
    typedef ControlUpdate<int> EncoderUpdate;

    const int POT_FIR_SIZE = 60;

    // Note: Ideal update frequency for this is about 1Khz
    class ControlManager
    {
        int EncoderState = 0;
        int EncoderValue = 0;

        // Encoder switch is the 9th button!
        int ButtonCounter[9] = {0};
        bool ButtonState[9] = {false};
        bool ButtonOutputValue[9] = {false};

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
        inline void UpdateEncoderState()
        {
            int prevState = EncoderState;

            int a = !digitalRead(PIN_ENC_A);
            int b = !digitalRead(PIN_ENC_B);
            if (EncoderState == 0 && a && !b) EncoderState = 1;
            else if (EncoderState == 0 && !a && b) EncoderState = 3;
            else if (EncoderState == 1 && a && b) EncoderState = 2;
            else if (EncoderState == 1 && !a && !b) EncoderState = 0;
            else if (EncoderState == 2 && !a && b) EncoderState = 3;
            else if (EncoderState == 2 && a && !b) EncoderState = 1;
            else if (EncoderState == 3 && !a && !b) EncoderState = 0;
            else if (EncoderState == 3 && a && b) EncoderState = 2;

            if (EncoderState == 1 && prevState == 0) EncoderValue--;
            else if (EncoderState == 2 && prevState == 1) EncoderValue--;
            else if (EncoderState == 3 && prevState == 2) EncoderValue--;
            else if (EncoderState == 0 && prevState == 3) EncoderValue--;
            else if (EncoderState == 3 && prevState == 0) EncoderValue++;
            else if (EncoderState == 2 && prevState == 3) EncoderValue++;
            else if (EncoderState == 1 && prevState == 2) EncoderValue++;
            else if (EncoderState == 0 && prevState == 1) EncoderValue++;
        }

        inline void UpdatePotAndButton(int idx)
        {
            int bitA = (idx & 0b001) > 0;
            int bitB = (idx & 0b010) > 0;
            int bitC = (idx & 0b100) > 0;

            digitalWrite(PIN_MUX_A, bitA);
            digitalWrite(PIN_MUX_B, bitB);
            digitalWrite(PIN_MUX_C, bitC);
            delayMicroseconds(2);

            auto btnVal = digitalRead(PIN_BTN_IN);
            UpdateButtonState(idx, btnVal);

            // Todo: Encoder switch handling
            if (idx < 4)
            {
                auto potVal = analogRead(PIN_POT_IN);
                UpdatePotState(idx, potVal);
            }
            else if (idx == 4)
            {
                // special case for encoder switch, which is analog #4, but stored as button #8
                auto encoderSwitch = digitalRead(PIN_POT_IN);
                UpdateButtonState(8, !encoderSwitch);
            }
        }
    private:
        inline void UpdatePotState(int pot, float newVal)
        {
            potStateIdx[pot] = (potStateIdx[pot] + 1) % POT_FIR_SIZE;
            int idxWrite = potStateIdx[pot];
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

        inline void UpdateButtonState(int idx, int value)
        {
            ButtonCounter[idx] += (-1 + 2*value);
            
            if (ButtonCounter[idx] < 0)
                ButtonCounter[idx] = 0;

            if (ButtonCounter[idx] > 15)
                ButtonCounter[idx] = 15;
            
            if (ButtonCounter[idx] > 12)
                ButtonState[idx] = true;
            
            if (ButtonCounter[idx] < 3)
                ButtonState[idx] = false;
        }

        inline ButtonUpdate GetButton(int idx)
        {
            if (idx < 0 || idx >= 8)
                return ButtonUpdate(false, false);

            auto val = ButtonState[idx];
            bool isNew = val != ButtonOutputValue[idx];
            ButtonOutputValue[idx] = val;
            return ButtonUpdate(val, isNew);
        }

        inline ButtonUpdate GetEncoderButton()
        {
            int idx = 8;
            auto val = ButtonState[idx];
            bool isNew = val != ButtonOutputValue[idx];
            ButtonOutputValue[idx] = val;
            return ButtonUpdate(val, isNew);
        }

        inline EncoderUpdate GetEncoderDelta()
        {
            int delta = EncoderValue / 4;
            EncoderValue -= delta * 4;
            return EncoderUpdate(delta, delta != 0);
        }
    };

    extern ControlManager controls;
}
