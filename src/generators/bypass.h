// #pragma once

// #include "generator.h"
// #include "menu.h"
// #include "modules/biquad.h"

// namespace Cyber
// {
//     class Bypass : public Generator
//     {
//         Menu menu;

//     public:
//         const char* Tabs[4] = {"", "", "", ""};
//         Bypass();
//         virtual const char** GetTabs() override { return Tabs; }

//         inline virtual Menu* GetMenu(int tab) override { return &menu; }
//         inline virtual void Process(GeneratorArgs args) override 
//         {
//             Utils::Copy(args.OutputLeft, args.InputLeft, args.Size);
//         }

//     public:
//         inline static GeneratorInfo GetInfo()
//         {
//             GeneratorInfo info;
//             info.DeveloperName = "Ghost Note Audio";
//             info.DisplayName = "Bypass";
//             info.GeneratorId = "Bypass";
//             info.Info = "Bypass";
//             info.Version = 1000;
//             return info;
//         }

//         inline static void SplashScreen(U8G2* display)
//         {
//             display->clearDisplay();
//             display->setDrawColor(1);
//             display->setFont(BIG_FONT);
//             display->setCursor(4, 36);
//             display->print("Bypass");
//         }
//     };
// }
