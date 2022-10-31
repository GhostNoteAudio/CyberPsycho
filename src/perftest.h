#include "cyberpsycho.h"

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
            if (opsPerSec > 10000000)
            {
                opsPerSec /= 1000000;
                multiplier = " M";
            }
            else if (opsPerSec > 10000)
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
        for (int i = 0; i < 100000; i++)
        {
            sum += tanhf(i);
        }
        summer += sum;
        pt.Stop();
        show(100000, "TanhfInt");

        LogInfof("-------------- END OF RESULT %d ------------", summer);
        delay(1000);
    }
}
