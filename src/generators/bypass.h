#pragma once

#include "generator.h"
#include "menu.h"
#include <fonts/font3.h>
#include "modules/biquad.h"

namespace Cyber
{
    class Bypass : public Generator
    {
        Menu menu;

    public:
        inline Bypass()
        {
            menu.CustomOnlyMode = true;
            menu.RenderCustomDisplayCallback = [](Adafruit_SH1106G* display)
            {
                display->clearDisplay();
                display->setCursor(16, 32);
                display->setTextSize(2);
                display->print("Bypassed");
            };

            menu.SetLength(0);
            menu.SelectedItem = 0;
            menu.TopItem = 0;
            menu.EnableSelection = false;
            menu.QuadMode = true;
        }

        inline virtual Menu* GetMenu() override { return &menu; }
        inline virtual void Process(GeneratorArgs args) override 
        {
            Utils::Copy(args.OutputLeft, args.InputLeft, args.Size);
        }

    public:
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Bypass";
            info.GeneratorId = "Bypass";
            info.Info = "Bypass";
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
            display->print("Bypass");
        }
    };
}
