#pragma once

#include "generator.h"
#include "menu.h"
#include <fonts/font3.h>
#include "modules/biquad.h"

namespace Cyber
{
    class EQShelf : public Generator
    {
        Modules::Biquad biquadLow;
        Modules::Biquad biquadHigh;
        Menu menu;

    public:
        EQShelf();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;

    private:
        float GetScaledParameter(int idx);

    public:
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "EQ Shelf";
            info.GeneratorId = "GNA-EQShelf";
            info.Info = "High/Low shelving Equaliser";
            info.Version = 1000;
            info.InsertEffect = true;
            return info;
        }

        inline static void SplashScreen(Adafruit_SH1106G* display)
        {
            display->clearDisplay();
            display->setTextColor(SH110X_WHITE);
            display->setFont(&AtlantisInternational_jen08pt7b);
            display->setTextSize(2);
            display->setCursor(0, 32);
            display->print("EQ Shelf");
        }
    };

}