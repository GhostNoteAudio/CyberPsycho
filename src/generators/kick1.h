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
        float phasor;
        bool currentGate;

        Modules::Envelope ampEnv;
        Modules::Envelope pitchEnv;
    public:
        Kick1();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;

    private:
        float GetScaledParameter(int idx);

    public:
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Kick One";
            info.GeneratorId = "GNA-Kick1";
            info.Info = "X0X-esque kick drum synth.";
            info.Version = 1000;
            return info;
        }

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