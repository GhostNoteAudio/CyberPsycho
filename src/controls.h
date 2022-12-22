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

    // Note: Ideal update frequency for this is about 1Khz
    class ControlManager
    {
        int EncoderState = 0;
        int EncoderValue = 0;

        int CurrentIdx = 0;

        // Encoder switch is the 9th button!
        int ButtonCounter[9] = {0};
        bool ButtonState[9] = {false};
        bool ButtonOutputValue[9] = {false};

        float PotPredictedValue[4] = {0.0};
        float PotOutputValue[4] = {0.0};
        uint PotFastMovementTimestamp[4] = {0};
        
        // clip the top and bottom range to ensure the pot can reach min and max reliably
        const int PotDeadSpace = 4;
        const float PotScaler = 1024.0 / (1024.0 - 2*PotDeadSpace);
        const float Pot10BitScale = 1.0 / 1023.0;

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

        inline void SetNextIndex()
        {
            CurrentIdx = (CurrentIdx + 1) % 8;

            int bitA = (CurrentIdx & 0b001) > 0;
            int bitB = (CurrentIdx & 0b010) > 0;
            int bitC = (CurrentIdx & 0b100) > 0;

            digitalWrite(PIN_MUX_A, bitA);
            digitalWrite(PIN_MUX_B, bitB);
            digitalWrite(PIN_MUX_C, bitC);
        }

        inline void UpdatePotAndButton()
        {
            auto btnVal = digitalRead(PIN_BTN_IN);
            UpdateButtonState(CurrentIdx, btnVal);

            // Todo: Encoder switch handling
            if (CurrentIdx < 4)
            {
                auto potVal = analogRead(PIN_POT_IN);
                UpdatePotState(CurrentIdx, potVal);
            }
            else if (CurrentIdx == 4)
            {
                // special case for encoder switch, which is analog #4, but stored as button #8
                auto encoderSwitch = digitalRead(PIN_POT_IN);
                UpdateButtonState(8, !encoderSwitch);
            }

            SetNextIndex();
        }

    private:
        inline void UpdatePotState(int pot, float newVal)
        {
            float alpha;
            float delta = fabsf(newVal - PotPredictedValue[pot]);

            if (delta > 10)
                PotFastMovementTimestamp[pot] = millis();

            if (PotFastMovementTimestamp[pot] + 250 > millis()) alpha = 0.2;
            else if (delta < 4) alpha = 0.005;
            else if (delta < 8) alpha = 0.05;
            else alpha = 0.2;

            PotPredictedValue[pot] = PotPredictedValue[pot] * (1-alpha) + newVal * alpha;
            //LogInfof("Delta: %.3f, Alpha: %.3f, PredVal: %.1f", delta, alpha, PotPredictedValue[pot]);
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
            float potVal = PotPredictedValue[pot];
            return ScalePot(potVal) * Pot10BitScale;
        }

        inline PotUpdate GetPot(int pot)
        {
            float val = ScalePot(PotPredictedValue[pot]);
            float currentVal = PotOutputValue[pot];
            float delta = fabsf(val - currentVal);
            bool isBoundary = val == 0 || val == 1023;

            // We take special care that we can reach the boundaries without problems
            if ((isBoundary && delta > 0) || delta > 4)
            {
                PotOutputValue[pot] = val;
                return PotUpdate(val * Pot10BitScale, true);
            }
            else
            {
                return PotUpdate(currentVal * Pot10BitScale, false);
            }
        }

        inline void UpdateButtonState(int idx, int value)
        {
            ButtonCounter[idx] += (-1 + 2*value);
            
            if (ButtonCounter[idx] < 0)
                ButtonCounter[idx] = 0;

            if (ButtonCounter[idx] > 4)
                ButtonCounter[idx] = 4;
            
            if (ButtonCounter[idx] > 3)
                ButtonState[idx] = true;
            
            if (ButtonCounter[idx] < 1)
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
