#include "Arduino.h"
#include "cyberpsycho.h"

using namespace Cyber;



void HandleAudioFunction(DataBuffer* data)
{
    Scope::ProcessScope(data);
    //auto fpData = inProcessor.ConvertToFp(data);
    //auto min = Utils::Min(fpData.Cv[3], fpData.Size);
    //auto max = Utils::Max(fpData.Cv[3], fpData.Size);
    //LogInfof("Min: %f - Max: %f", min, max)

    for (int i = 0; i < data->Size; i++)
        data->Out[3][i] = data->Mod[3][i];

    //delayMicroseconds(600);
}

void setup()
{
    Serial.begin(9600);
    //while(!Serial) {}
    Serial.println("Starting...");

    Menus::Init();
    displayManager.Init();
    Menus::ActiveMenu = &Menus::initMenu;

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

        //auto py = GetPerfYield();
        //auto pa = GetPerfAudio();
        //auto pi = GetPerfIo();
        float cpuLoad = GetCpuLoad();
        //LogInfof("Audio Time : %f %f %f", pa->Period(), pa->PeriodAvg(), pa->PeriodDecay());
        //LogInfof("Yield Time : %f %f %f", py->Period(), py->PeriodAvg(), py->PeriodDecay());
        //LogInfof("IO Time : %f %f %f", pi->Period(), pi->PeriodAvg(), pi->PeriodDecay());
        LogInfof("CPU Load: %.3f", cpuLoad);
        
    }

    YieldAudio();

    if (updateState.Go())
    {
        controls.UpdatePotState(0);
        controls.UpdatePotState(1);
        controls.UpdatePotState(2);
        controls.UpdatePotState(3);
        controls.UpdateButtonState();
    }

    if (updateMenu.Go())
    {
        auto p0 = controls.GetPot(0);
        auto p1 = controls.GetPot(1);
        auto p2 = controls.GetPot(2);
        auto p3 = controls.GetPot(3);
        auto btn0 = controls.GetButton(0);
        auto btn1 = controls.GetButton(1);
        auto btn2 = controls.GetButton(2);
        auto btn3 = controls.GetButton(3);

        if (p0.IsNew)
            Menus::ActiveMenu->HandlePot(0, p0.Value);
        if (p1.IsNew)
            Menus::ActiveMenu->HandlePot(1, p1.Value);
        if (p2.IsNew)
            Menus::ActiveMenu->HandlePot(2, p2.Value);
        if (p3.IsNew)
            Menus::ActiveMenu->HandlePot(3, p3.Value);

        if (btn0)
            Menus::ActiveMenu->HandleSwitch(0, btn0);
        if (btn1)
            Menus::ActiveMenu->HandleSwitch(1, btn1);
        if (btn2)
            Menus::ActiveMenu->HandleSwitch(2, btn2);
        if (btn3)
            Menus::ActiveMenu->HandleSwitch(3, btn3);

        YieldAudio();
        displayManager.Render();
    }

    YieldAudio();

    if (i2cMaster.finished())
    {
        displayManager.Transfer();
    }
}
