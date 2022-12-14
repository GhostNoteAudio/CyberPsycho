#pragma once

#include "generator.h"
#include "menu.h"
#include "modules/percussionEnvelope.h"

namespace Cyber
{

    class Kick1 : public Generator
    {
        const char* Tabs[4] = {"KICK", "ENV", "", ""};
        Menu menu;
        float phasor;
        bool currentGate;

        Modules::PercussionEnvelope ampEnv;
        Modules::PercussionEnvelope pitchEnv;
    public:
        Kick1();
        virtual const char** GetTabs() override { return Tabs; }
        virtual Menu* GetMenu() override;
        virtual inline void SetTab(int tab) override;
        virtual int GetModSlots();
        virtual void GetModSlotName(int idx, char* dest);
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

        inline static void SplashScreen(U8G2* display)
        {
            display->clearDisplay();
            display->setDrawColor(1);
            display->setFont(BIG_FONT);
            display->setCursor(4, 36);
            display->print("Kick 1");
        }
    };

}