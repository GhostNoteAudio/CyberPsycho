// #pragma once

// #include "generator.h"
// #include "menu.h"
// #include "modules/biquad.h"
// #include "modules/filterCascade.h"

// namespace Cyber
// {
//     class MultimodeFilter : public Generator
//     {
//         Menu menu;
//         Modules::Biquad biq;
//         Modules::FilterCascade cascade;
        
//     public:
//         MultimodeFilter();
//         virtual Menu* GetMenu() override;
//         virtual void Process(GeneratorArgs args) override;

//     private:
//         float GetScaledParameter(int idx);

//     public:
//         inline static GeneratorInfo GetInfo()
//         {
//             GeneratorInfo info;
//             info.DeveloperName = "Ghost Note Audio";
//             info.DisplayName = "Multimode Filter";
//             info.GeneratorId = "GNA-MultimodeFilter";
//             info.Info = "Multi-mode filter.";
//             info.Version = 1000;
//             info.InsertEffect = true;
//             return info;
//         }

//         inline static void SplashScreen(U8G2* display)
//         {
//             display->clearDisplay();
//             display->setDrawColor(1);
//             display->setFont(BIG_FONT);
//             YieldAudio();
//             display->setCursor(4, 26);
//             display->print("Multi Mode");
//             YieldAudio();
//             display->setCursor(4, 46);
//             display->print("Filter");
//         }
//     };
// }
