#pragma once

#include "generator.h"
#include "menu.h"
#include <fonts/font3.h>
#include "modules/envelope.h"

namespace Cyber
{

    class Kick1 : public Generator
    {
        Menu menu;
        int index;
        float phasor;
        bool currentGate;

        //Modules::Envelope ampEnv;
    public:
        Kick1();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;
        virtual void ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1) override;
        virtual void ProcessOffline() override;

    private:
        float GetScaledParameter(int idx);

        inline float Boost(float s, float gain)
        {
            return tanhf(s * gain);
            // s = s * gain;
            // s = s < -1 ? -1 : s > 1 ? 1 : s;
            // return s;
        }

        inline float Fold(float s, float gain)
        {
            return sinf(s * gain);
        }

        inline void Trigger()
        {
            index = 0;
            phasor = 0;
        }

    public:
        inline static void SplashScreen(Adafruit_SH1106G* display)
        {
            display->clearDisplay();
            display->setTextColor(SH110X_WHITE);
            display->setFont(&AtlantisInternational_jen08pt7b);
            display->setTextSize(2);
            display->setCursor(10, 32);
            display->print("Kick 1");
        }
    };

}