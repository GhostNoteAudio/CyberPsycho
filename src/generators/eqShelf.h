#pragma once

#include "generator.h"
#include "menu.h"
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

        inline static void SplashScreen(U8G2* display)
        {
            display->clearDisplay();
            display->setDrawColor(1);
            display->setFont(BIG_FONT);
            display->setCursor(4, 36);
            display->print("EQ Shelf");
        }
    };

}