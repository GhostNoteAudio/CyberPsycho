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
        GeneratorInfo Info[T];
        std::function<void(Adafruit_SH1106G*)> SplashScreenBuilders[T];

        template<class TGen>
        inline void Add()
        {
            Factories[Count] = []{return new TGen();};
            Info[Count] = TGen::GetInfo();
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
