#pragma once
#include "Adafruit_GFX.h"
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
        std::function<Adafruit_GFX*(void)> SplashScreenBuilders[T];

        inline void Add(std::function<Generator*(void)> factory, const char* name, std::function<Adafruit_GFX*(void)> splashScreenBuilder)
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

    inline void RegisterGenerator(std::function<Generator*(void)> factory, const char* name, std::function<Adafruit_GFX*(void)> splashScreenBuilder)
    {
        generatorRegistry.Add(factory, name, splashScreenBuilder);
    }
}
