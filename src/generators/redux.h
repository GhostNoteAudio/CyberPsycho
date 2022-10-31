#pragma once

#include "generator.h"
#include "menu.h"
#include <fonts/font3.h>
#include "modules/biquad.h"

namespace Cyber
{
    class Redux : public Generator
    {
        float phasor = 0;
        float sample = 0;
        Modules::Biquad biquad;
        Menu menu;

    public:
        Redux();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;

    private:
        float GetScaledParameter(int idx);

    public:
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Redux";
            info.GeneratorId = "GNA-Redux";
            info.Info = "Bitcrusher and Samplerate reducer";
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
            display->print("Redux");
        }
    };

}