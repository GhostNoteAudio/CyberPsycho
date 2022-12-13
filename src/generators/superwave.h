#pragma once

#include "generator.h"
#include "menu.h"
#include "modules/biquad.h"

namespace Cyber
{

    class Superwave : public Generator
    {
        Menu menu;
        float pitch = 48;
        float gainAdjust = 1.0;
        uint32_t phasorL[7] = {0};
        uint32_t phasorR[7] = {0};
        float volumes[7] = {0};
        Modules::Biquad biqL, biqR;
        
    public:
        Superwave();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;
        void Update(float pitchHz = -1);

    private:
        float GetScaledParameter(int idx);

    public:
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Superwave";
            info.GeneratorId = "GNA-Superwave";
            info.StereoOutSupport = true;
            info.Info = "Supersaw oscillator with tunable parameters.";
            info.Version = 1000;
            info.InsertEffect = false;
            return info;
        }

        inline static void SplashScreen(U8G2* display)
        {
            display->clearDisplay();
            display->setDrawColor(1);
            display->setFont(BIG_FONT);
            display->setCursor(4, 36);
            display->print("Superwave");
        }
    };

}