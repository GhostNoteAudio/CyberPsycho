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
            AR1shot = 1,
            //AHR = 2,
            //ADSR = 3,
            //AHDRS = 4
        };

    public:
        int Stage = 4; // 0=attack, 1=hold, 2=decay, 3=sustain, 4=release
        int SamplesSinceStage = 1000000000; // how many samples have been processed since entering the current stage
        bool currentGate = false;
        float AttackInc;
        float DecayInc;
        float ReleaseInc;

    public:
        EnvMode Mode = EnvMode::AR;
        CurveMode AttackMode = CurveMode::Linear;
        CurveMode DecayMode = CurveMode::Linear;
        float AttackSamples = 1000;
        float HoldSamples = 0;
        float DecaySamples = 1000;
        float SustainLevel = 0.5;
        float ReleaseSamples = 1000;
        bool ResetOnTrig = true;

        float Output = 0;

        void UpdateParams()
        {
            AttackInc = 1.0 / AttackSamples;
            DecayInc = 1.0 / DecaySamples;
            ReleaseInc = 1.0 / ReleaseSamples;
        }

    private:
        void ProcessAr(bool trig, bool gate)
        {
            if (trig)
            {
                Stage = 0;
                if (ResetOnTrig)
                    Output = 0;
            }

            if (Stage == 4)
            {
                Output -= ReleaseInc;
                if (Output < 0)
                    Output = 0;
            }

            if (Stage == 0)
            {
                LogInfo("Bang in stage 0");
                if (gate || Mode == EnvMode::AR1shot)
                {
                    Output += AttackInc;
                    if (Output >= 1)
                    {
                        Output = 1;
                        Stage = 4;
                    }
                }
                else
                {
                    Stage = 4;
                    Output -= ReleaseInc;
                    if (Output < 0)
                        Output = 0;
                }
            }
        }

    public:
        float Process(bool gate)
        {
            bool trig = !this->currentGate && gate;
            int stageOrig = Stage;

            /*if (trig)
            {
                LogInfof("Trig! gate: %d, currentGate: %d", gate, this->currentGate);
            }*/

            if (Mode == EnvMode::AR || Mode == EnvMode::AR1shot)
                ProcessAr(trig, gate);

            this->currentGate = gate;

            /*if (trig)
            {
                LogInfof("I just set currentGate to %d, using gate %d", currentGate, gate);
            }*/

            if (stageOrig == Stage)
                SamplesSinceStage++;
            else
                SamplesSinceStage = 1;
            
            return Output;
        }
    };
}
