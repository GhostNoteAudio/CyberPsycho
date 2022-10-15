#include "Arduino.h"
#include "cyberpsycho.h"

using namespace Cyber;
Menu m;

void BuildMenu()
{
    m.Captions[0] = "Param 0";
    m.Captions[1] = "Param 1";
    m.Captions[2] = "Param 2";
    m.Captions[3] = "Param 3";
    m.Captions[4] = "Param 4";
    m.Captions[5] = "Param 5";
    m.Captions[6] = "Param 6";

    m.Values[0] = 0.0;
    m.Values[1] = 1.0;
    m.Values[2] = 2.0;
    m.Values[3] = 3.0;
    m.Values[4] = 4.0;
    m.Values[5] = 5.0;
    m.Values[6] = 6.0;

    m.Formatters[3] = [](float v, char* s) { v < 0.5 ? sprintf(s, "Off") : sprintf(s, "On"); };

    m.SetLength(7);
    m.SelectedItem = 3;
    m.TopItem = 0;
    m.EnableSelection = true;
    m.QuadMode = true;
}

void HandleAudioFunction(DataBuffer* data)
{
    auto fpData = inProcessor.ConvertToFp(data);
    //auto min = Utils::Min(fpData.Cv[3], fpData.Size);
    //auto max = Utils::Max(fpData.Cv[3], fpData.Size);
    //LogInfof("Min: %f - Max: %f", min, max)

    for (int i = 0; i < data->Size; i++)
        data->Out[3][i] = data->Mod[3][i];

    //delayMicroseconds(240);
}

void setup()
{
    Serial.begin(9600);
    //while(!Serial) {}
    Serial.println("Starting...");

    BuildMenu();
    menuManager.Init();
    menuManager.ActiveMenu = &m;

    audio.Init();
    audio.StartProcessing();
    Serial.println("Done");

    i2cMaster.begin(1000000);
    HandleAudioCb = HandleAudioFunction;
}

PeriodicExecution execPrint(1000);
PeriodicExecution execPrintFast(100);
PeriodicExecution updateState(1);
PeriodicExecution updateMenu(10);

void loop()
{
    YieldAudio();

    if (execPrint.Go())
    {
        if (audio.BufferUnderrun)
        {
            LogInfo("Buffer Underrun!!");
            audio.BufferUnderrun = false;
        }

        auto py = GetPerfYield();
        auto pa = GetPerfAudio();
        auto pi = GetPerfIo();
        //LogInfof("CPU load: %.2f%%", Timers::GetCpuLoad()*100);

        LogInfof("Audio Time : %f %f %f", pa->Period(), pa->PeriodAvg(), pa->PeriodDecay());
        LogInfof("Yield Time : %f %f %f", py->Period(), py->PeriodAvg(), py->PeriodDecay());
        LogInfof("IO Time : %f %f %f", pi->Period(), pi->PeriodAvg(), pi->PeriodDecay());
        float cpuLoad = GetCpuLoad();
        LogInfof("CPU Load: %.3f", cpuLoad);
        
    }

    YieldAudio();

    if (updateState.Go())
    {
        controls.UpdatePotState(0);
        controls.UpdatePotState(1);
    }

    if (updateMenu.Go())
    {
        menuManager.HandlePotUpdate(0, controls.GetPot(0).Value);
        menuManager.HandlePotUpdate(1, controls.GetPot(1).Value);
        menuManager.Render();
    }

    YieldAudio();

    if (i2cMaster.finished())
    {
        menuManager.Transfer();
    }
}
