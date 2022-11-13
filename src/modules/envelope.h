#pragma once
#include "logging.h"
#include "fast_curves.h"

namespace Modules
{
    class Envelope
    {
    public:
        enum class EnvCurve
        {
            Linear = 0,
            Exp = 1,
        };

        enum class EnvMode
        {
            AR = 0,
            AHR = 1,
            ADSR = 2,
            AHDSR = 3
        };

        const int STAGE_ATK = 0;
        const int STAGE_HOL = 1;
        const int STAGE_DEC = 2;
        const int STAGE_SUS = 3;
        const int STAGE_REL = 4;

    private:
        int Stage = STAGE_REL;
        bool currentGate = false;
        
        float AttackInc;
        float DecayInc;
        float ReleaseInc;

        int updateCounter = 0;
        float Output = 0;
        int HoldCount = 0;
        float ReleaseScaler = 1;

    public:
        EnvMode Mode = EnvMode::AR;
        EnvCurve AttackCurve = EnvCurve::Linear;
        EnvCurve DecayCurve = EnvCurve::Linear;
        EnvCurve ReleaseCurve = EnvCurve::Linear;
        float AttackSamples = 1000;
        float HoldSamples = 0;
        float DecaySamples = 1000;
        float SustainLevel = 0.5;
        float ReleaseSamples = 1000;
        bool ResetOnTrig = true; // if true, will always reset env output to zero on trigger
        bool OneShot = false; // if true, will latch gate until the end of the release phase is reached. Useful for permission sounds

        inline int GetCurrentStage()
        {
            return Stage;
        }

        inline float GetOutput()
        {
            if (Stage == STAGE_ATK)
                return AttackCurve == EnvCurve::Exp ? Cyber::FastCurves::Read(Cyber::FastCurves::Resp2Dec, Output) : Output;
            if (Stage == STAGE_HOL)
                return 1;
            if (Stage == STAGE_DEC)
            {
                float decOut = DecayCurve == EnvCurve::Exp ? Cyber::FastCurves::Read(Cyber::FastCurves::Resp2Dec, Output) : Output;
                return decOut * (1-SustainLevel) + SustainLevel;
            }
            if (Stage == STAGE_SUS)
                return SustainLevel;
            if (Stage == STAGE_REL)
                return ReleaseScaler * (ReleaseCurve == EnvCurve::Exp ? Cyber::FastCurves::Read(Cyber::FastCurves::Resp2Dec, Output) : Output);

            return 0;
        }

        inline void UpdateParams()
        {
            AttackInc = 1.0f / AttackSamples;
            DecayInc = 1.0f / DecaySamples;
            ReleaseInc = 1.0f / ReleaseSamples;
        }

    private:
        inline void ProcessStage()
        {
            if (Stage == STAGE_ATK)
            {
                Output += AttackInc;
                if (Output >= 1)
                {
                    Output = 1;
                    if (Mode == EnvMode::AR)
                    {
                        ReleaseScaler = 1.0f;
                        Stage = STAGE_REL;
                    }
                    else if (Mode == EnvMode::AHR || Mode == EnvMode::AHDSR)
                    {
                        Stage = STAGE_HOL;
                        HoldCount = 0;
                    }
                    else if (Mode == EnvMode::ADSR)
                    {
                        Stage = STAGE_DEC;
                    }
                }
            }
            else if (Stage == STAGE_HOL)
            {
                Output = 1;
                HoldCount++;

                if (HoldCount >= HoldSamples)
                {
                    ReleaseScaler = 1.0f;
                    if (Mode == EnvMode::AHR)
                        Stage = STAGE_REL;
                    else if (Mode == EnvMode::AHDSR)
                        Stage = STAGE_DEC;
                }
            }
            else if (Stage == STAGE_DEC)
            {
                Output -= DecayInc;
                if (Output <= 0)
                {
                    Output = 0;
                    Stage = STAGE_SUS;
                }
            }
            else if (Stage == STAGE_SUS)
            {
                Output = SustainLevel;
            }
            else if (Stage == STAGE_REL)
            {
                Output -= ReleaseInc;
                if (Output < 0)
                    Output = 0;
            }
        }

        inline float GetValueForAttackReset()
        {
            // When going back to the attack stage, we can't just take the current output value and assign it as the Attack value,
            // because the stage could be using different curves. This attempts to calculate the right value.

            // Todo: Do properly! Currently only works for linear attack!
            // Use bisect method, maybe?
            return GetOutput();
        }

    public:
        inline float Process(bool gate)
        {
            if (updateCounter >= 16)
            {
                UpdateParams();
                updateCounter = 0;
            }

            bool trig = !this->currentGate && gate;

            if (trig)
            {
                if (ResetOnTrig)
                    Output = 0;
                else
                    Output = GetValueForAttackReset();

                Stage = STAGE_ATK;
            }

            if (!gate && !OneShot && Stage != STAGE_REL)
            {
                ReleaseScaler = GetOutput();
                Output = 1;
                Stage = STAGE_REL;
            }

            ProcessStage();
            this->currentGate = gate;
            updateCounter++;
            return GetOutput();
        }
    };
}
