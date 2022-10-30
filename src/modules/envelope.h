#pragma once
#include "logging.h"

namespace Modules
{
    class Envelope
    {
    public:
        enum class CurveMode
        {
            Linear = 0,
            Exp = 1,
        };

        enum class EnvMode
        {
            AR = 0,
            //AHR = 1,
            //ADSR = 2,
            //AHDRS = 3
        };

    private:
        float ExpMin = 0.001; // -60db
        float ExpScaler = (1.0/(1.0 - 0.001)); // used to scale output from [0.001, 1] to [0, 1]

        int Stage = 4; // 0=attack, 1=hold, 2=decay, 3=sustain, 4=release
        int SamplesSinceStage = 1000000000; // how many samples have been processed since entering the current stage
        bool currentGate = false;
        
        // Linear mode
        float AttackInc;
        float DecayInc;
        float ReleaseInc;

        // Exp mode
        float AttackMultiplier;
        float DecayMultiplier;
        float ReleaseMultiplier;

        float Output = 0;

    public:
        EnvMode Mode = EnvMode::AR;
        CurveMode AttackMode = CurveMode::Linear;
        CurveMode DecayMode = CurveMode::Linear;
        CurveMode ReleaseMode = CurveMode::Linear;
        float AttackSamples = 1000;
        float HoldSamples = 0;
        float DecaySamples = 1000;
        float SustainLevel = 0.5;
        float ReleaseSamples = 1000;
        bool ResetOnTrig = true; // if true, will always reset env output to zero on trigger
        bool OneShot = false; // if true, will latch gate until the end of the release phase is reached. Useful for permission sounds

        int GetCurrentStage()
        {
            return Stage;
        }

        float GetOutput()
        {
            bool scale = false;
            if (Stage == 0 && AttackMode == CurveMode::Exp) scale = true;
            if (Stage == 2 && DecayMode == CurveMode::Exp) scale = true;
            if (Stage == 4 && ReleaseMode == CurveMode::Exp) scale = true;
            return scale ? (Output - ExpMin) * ExpScaler : Output;
        }

        void UpdateParams(float minimumExpDb = -60)
        {
            const float div20 = (1.0 / 20.0);

            ExpMin = pow10f(minimumExpDb * div20);
            ExpScaler = 1.0f/(1.0f - ExpMin);
            minimumExpDb = -minimumExpDb;

            AttackInc = 1.0f / AttackSamples;
            DecayInc = 1.0f / DecaySamples;
            ReleaseInc = 1.0f / ReleaseSamples;

            // calculate the multiplier M, as so:
            // ExpMin * M^Samples = 1.0  - for attack
            // or
            // 1.0 * M^Samples = ExpMin  - for decay and release
            // AttackMultiplier = pow10f((minimumExpDb/AttackSamples) * div20);
            // DecayMultiplier = 1.0f / pow10f((minimumExpDb/DecaySamples) * div20);
            // ReleaseMultiplier = 1.0f / pow10f((minimumExpDb/ReleaseSamples) * div20);
            AttackMultiplier = pow10f((minimumExpDb*AttackInc) * div20);
            DecayMultiplier = 1.0f / pow10f((minimumExpDb*DecayInc) * div20);
            ReleaseMultiplier = 1.0f / pow10f((minimumExpDb*ReleaseInc) * div20);
        }

    private:
        void ProcessAr()
        {
            if (Stage == 4) // Release
            {
                if (ReleaseMode == CurveMode::Linear)
                {
                    Output -= ReleaseInc;
                    if (Output < 0)
                        Output = 0;
                }
                else
                {
                    Output *= ReleaseMultiplier;
                    if (Output < ExpMin)
                        Output = ExpMin;
                }
            }

            if (Stage == 0) // Attack
            {
                if (AttackMode == CurveMode::Linear)
                    Output += AttackInc;
                else
                    Output *= AttackMultiplier;

                if (Output >= 1)
                {
                    Output = 1;
                    Stage = 4;
                }
            }
        }

    public:
        float Process(bool gate)
        {
            bool trig = !this->currentGate && gate;
            int stageOrig = Stage;

            if (trig)
            {
                Stage = 0;
                if (ResetOnTrig)
                    Output = AttackMode == CurveMode::Linear ? 0 : ExpMin;
            }

            if (!gate && !OneShot)
            {
                Stage = 4;
            }

            if (Mode == EnvMode::AR || Mode == EnvMode::AR)
                ProcessAr();

            this->currentGate = gate;

            if (stageOrig == Stage)
                SamplesSinceStage++;
            else
                SamplesSinceStage = 1;
            
            return GetOutput();
        }
    };
}
