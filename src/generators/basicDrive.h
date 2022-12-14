// #pragma once

// #include "generator.h"
// #include "menu.h"
// #include "modules/biquad.h"

// namespace Cyber
// {
//     class BasicDrive : public Generator
//     {
//         Modules::Biquad biquadHp;
//         Modules::Biquad biquadLp;
//         Menu menu;

//     public:
//         BasicDrive();
//         virtual Menu* GetMenu() override;
//         virtual void Process(GeneratorArgs args) override;

//     private:
//         float GetScaledParameter(int idx);

//     public:
//         inline static GeneratorInfo GetInfo()
//         {
//             GeneratorInfo info;
//             info.DeveloperName = "Ghost Note Audio";
//             info.DisplayName = "Basic Drive";
//             info.GeneratorId = "GNA-BasicDrive";
//             info.Info = "Simple Drive/Distortion effect";
//             info.Version = 1000;
//             info.InsertEffect = true;
//             return info;
//         }

//         inline static void SplashScreen(U8G2* display)
//         {
//             display->clearDisplay();
//             display->setDrawColor(1);
//             display->setFont(BIG_FONT);
//             display->setCursor(4, 36);
//             display->print("Basic Drive");
//         }
//     };

// }