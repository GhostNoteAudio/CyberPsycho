#include "cyberpsycho.h"
#include "arm_math.h"

namespace Cyber
{
    inline void RunBenchmark()
    {
        PerfTimer pt;
        float summer = 0;
        float sum = 0;

        auto show = [&pt](int ops, const char* label)
        { 
            float opsPerSec = ops / pt.Period() * 1000000;
            const char* multiplier = "";
            if (opsPerSec > 1000000)
            {
                opsPerSec /= 1000000;
                multiplier = " M";
            }
            else if (opsPerSec > 1000)
            {
                opsPerSec /= 1000;
                multiplier = " K";
            }
            
            LogInfof("%s :: ops per sec: %.2f%s", label, opsPerSec, multiplier);
        };

        // ---------------------------------------------
        pt.Start();
        sum = 0;
        for (int i = 0; i < 1000000; i++)
        {
            sum += 1.0f;
        }
        summer += sum;
        pt.Stop();
        show(1000000, "FloatAcc");

        // ---------------------------------------------
        pt.Start();
        sum = 0;
        for (int i = 0; i < 1000000; i++)
        {
            float a = i;
            sum += a;
        }
        summer += sum;
        pt.Stop();
        show(1000000, "Int2FloatAcc");

        // ---------------------------------------------
        pt.Start();
        sum = 0.00001;
        for (int i = 0; i < 100000; i++)
        {
            sum *= 1.001f;
        }
        summer += sum;
        pt.Stop();
        show(100000, "FloatMult");

        summer /= 1000000000;

        // ---------------------------------------------
        pt.Start();
        sum = 1000000000;
        for (int i = 0; i < 100000; i++)
        {
            sum /= 1.000001f;
        }
        summer += sum;
        pt.Stop();
        show(100000, "FloatDiv");

        // ---------------------------------------------
        pt.Start();
        sum = 0;
        for (int i = 0; i < 10000; i++)
        {
            sum += sinf(i);
        }
        summer += sum;
        pt.Stop();
        show(10000, "SinfInt");

        // ---------------------------------------------
        pt.Start();
        sum = 0;
        for (int i = 0; i < 10000; i++)
        {
            sum += arm_sin_f32(i);
        }
        summer += sum;
        pt.Stop();
        show(10000, "arm_sin_f32__Int");

        // ---------------------------------------------
        pt.Start();
        sum = 0;
        for (int i = 0; i < 10000; i++)
        {
            sum += Utils::SinLut(i);
        }
        summer += sum;
        pt.Stop();
        show(10000, "SinLut");

        // ---------------------------------------------
        pt.Start();
        sum = 0;
        for (int i = 0; i < 100000; i++)
        {
            sum += tanhf(i);
        }
        summer += sum;
        pt.Stop();
        show(100000, "TanhfInt");

        // ---------------------------------------------
        pt.Start();
        sum = 0;
        for (int i = 0; i < 1000; i++)
        {
            float sum2 = 0;
            for (int j = 0; j < 128; j++)
            {
                sum2 += Utils::Note2hz(j);
            }
            
            sum += 0.0000001 * sum2;
        }
        summer += sum;
        pt.Stop();
        show(128000, "Note2Hz");

        // ---------------------------------------------
        pt.Start();
        sum = 0;
        for (int i = 0; i < 1000; i++)
        {
            float sum2 = 0;
            for (int j = 0; j < 128; j++)
            {
                sum2 += Utils::Note2HzLut(j);
            }
            
            sum += 0.0000001 * sum2;
        }
        summer += sum;
        pt.Stop();
        show(128000, "Note2HzLut");

        LogInfof("-------------- END OF RESULT %d ------------", summer);
        delay(1000);
    }
}
