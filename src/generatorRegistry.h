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

        inline void Add(std::function<Generator*(void)> factory, const char* name, std::function<void(Adafruit_SH1106G*)> splashScreenBuilder)
        {
            Factories[Count] = factory;
            Names[Count] = name;
            SplashScreenBuilders[Count] = splashScreenBuilder;
            Count++;
        }

        inline Generator* CreateInstance(int idx)
        {
            return Factories[idx]();
        }
    };

    extern GeneratorRegistry<100> generatorRegistry;

    inline void RegisterGenerator(std::function<Generator*(void)> factory, const char* name, std::function<void(Adafruit_SH1106G*)> splashScreenBuilder)
    {
        generatorRegistry.Add(factory, name, splashScreenBuilder);
    }
}
