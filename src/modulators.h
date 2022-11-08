#pragma once
#include "constants.h"
#include "modules/envelope.h"
#include "stdint.h"
#include "generator.h"

namespace Cyber
{
    class Modulators
    {
        Modules::Envelope env1;
        Modules::Envelope env2;

    public:
        float OutEnv1[BUFFER_SIZE];
        float OutEnv2[BUFFER_SIZE];
        float OutLfo1[BUFFER_SIZE];
        float OutLfo2[BUFFER_SIZE];

        Modulators()
        {
            env1.Mode = Modules::Envelope::EnvMode::ADSR;
            env1.ReleaseCurve = Modules::Envelope::EnvCurve::Exp;
            env1.AttackSamples = 40000;
            env1.DecaySamples = 10000;
            env1.SustainLevel = 0.5;
            env1.ReleaseSamples = 80000;

            env2.AttackSamples = 1000;
            env2.ReleaseSamples = 80000;

            env1.UpdateParams();
            env2.UpdateParams();
        }

        void Process(GeneratorArgs args)
        {
            for (int i = 0; i < args.Size; i++)
            {
                OutEnv1[i] = env1.Process(args.Gate[i]);
                //OutEnv2[i] = env2.Process(args.Gate[i]);
            }
        }
    };
}
