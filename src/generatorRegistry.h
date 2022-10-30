#pragma once
#include <Adafruit_SH110X.h>
#include "generator.h"

namespace Cyber
{
    template<int T>
    class GeneratorRegistry
    {
    public:
        int Count;
        std::function<Generator*(void)> Factories[T];
        const char* Names[T];
        std::function<void(Adafruit_SH1106G*)> SplashScreenBuilders[T];

        // inline void Add(std::function<Generator*(void)> factory, const char* name, std::function<void(Adafruit_SH1106G*)> splashScreenBuilder)
        // {
        //     Factories[Count] = factory;
        //     Names[Count] = name;
        //     SplashScreenBuilders[Count] = splashScreenBuilder;
        //     Count++;
        // }

        template<class TGen>
        inline void Add()
        {
            Factories[Count] = []{return new TGen();};
            Names[Count] = TGen::GetName();
            SplashScreenBuilders[Count] = [](Adafruit_SH1106G* display){TGen::SplashScreen(display);};
            Count++;
        }

        inline Generator* CreateInstance(int idx)
        {
            return Factories[idx]();
        }
    };

    extern GeneratorRegistry<100> generatorRegistry;
}
