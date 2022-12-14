#pragma once
#include <U8g2lib.h>
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
        std::function<void(U8G2*)> SplashScreenBuilders[T];

        template<class TGen>
        inline void Add()
        {
            Factories[Count] = []{return new TGen();};
            Info[Count] = TGen::GetInfo();
            SplashScreenBuilders[Count] = [](U8G2* display){TGen::SplashScreen(display);};
            Count++;
        }

        inline void DeleteInstance(Generator* gen)
        {
            delete gen;
        }

        inline Generator* CreateInstanceById(const char* genId)
        {
            for (int i = 0; i < Count; i++)
            {
                if (strcmp(Info[i].GeneratorId, genId) == 0)
                {
                    return CreateInstance(i);
                }
            }
            return 0;
        }

        inline Generator* CreateInstance(int idx)
        {
            auto instance = Factories[idx]();
            instance->GenIndex = idx;
            return instance;
        }
    };

    extern GeneratorRegistry<100> generatorRegistry;
}
