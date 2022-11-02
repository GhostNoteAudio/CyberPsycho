#pragma once

#include "generator.h"
#include "menu.h"
#include <fonts/font3.h>
#include "modules/biquad.h"

namespace Cyber
{

    class Superwave : public Generator
    {
        Menu menu;
        float pitch = 48;
        float gainAdjust = 1.0;
        uint32_t phasor[7] = {0};
        float volumes[7] = {0};
        Modules::Biquad biq;
        
    public:
        Superwave();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;
        void Reset(float pitchHz = -1);

    private:
        float GetScaledParameter(int idx);

    public:
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Superwave";
            info.GeneratorId = "GNA-Superwave";
            info.Info = "Supersaw oscillator with tunable parameters.";
            info.Version = 1000;
            info.InsertEffect = false;
            return info;
        }

        inline static void SplashScreen(Adafruit_SH1106G* display)
        {
            display->clearDisplay();
            display->setTextColor(SH110X_WHITE);
            display->setFont(&AtlantisInternational_jen08pt7b);
            display->setTextSize(2);
            display->setCursor(0, 32);
            display->print("Superwave");
        }
    };

}