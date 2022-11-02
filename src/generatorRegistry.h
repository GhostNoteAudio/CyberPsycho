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

        inline int GetInsertFxCount()
        {
            int counter = 0;
            for (int i = 0; i < Count; i++)
            {
                if (Info[i].InsertEffect)
                    counter++;
            }
            return counter;
        }

        inline int GetNextInsertFxIndex(int startIdx)
        {
            for (int i = startIdx+1; i < Count; i++)
            {
                if (Info[i].InsertEffect)
                    return i;
            }
            return -1;
        }

        inline int GetPrevInsertFxIndex(int startIdx)
        {
            for (int i = startIdx - 1; i >= 0 ; i--)
            {
                if (Info[i].InsertEffect)
                    return i;
            }
            return -1;
        }
    };

    extern GeneratorRegistry<100> generatorRegistry;
}
