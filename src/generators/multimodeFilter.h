#pragma once

#include "generator.h"
#include "menu.h"
#include <fonts/font3.h>
#include "modules/biquad.h"
#include "modules/filterCascade.h"

namespace Cyber
{
    class MultimodeFilter : public Generator
    {
        Menu menu;
        Modules::Biquad biq;
        Modules::FilterCascade cascade;
        
    public:
        MultimodeFilter();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;

    private:
        float GetScaledParameter(int idx);

    public:
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Multimode Filter";
            info.GeneratorId = "GNA-MultimodeFilter";
            info.Info = "Multi-mode filter.";
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
            display->setCursor(0, 20);
            display->print("MultiMode");
            display->setCursor(20, 40);
            display->print("Filter");
        }
    };
}
