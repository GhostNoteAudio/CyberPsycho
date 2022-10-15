#include "Arduino.h"
#include "menu_manager.h"
#include "menu.h"
#include "controls.h"
#include "audio_io.h"
#include "counter.h"
#include "logging.h"
#include "input_processor.h"
#include "utils.h"
#include <i2c_driver_wire.h>
#include "periodic_execution.h"
//#include "audio_yield.h"

InputProcessor inProcessor;
AudioIo audio;
Menu m;
MenuManager menuManager;
ControlManager controls;

extern I2CMaster& master;

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

    Wire.setClock(1000000);
}

void HandleAudio(DataBuffer* data)
{
    auto fpData = inProcessor.ConvertToFp(data);
    //auto min = Utils::Min(fpData.Cv[3], fpData.Size);
    //auto max = Utils::Max(fpData.Cv[3], fpData.Size);
    //LogInfof("Min: %f - Max: %f", min, max)

    for (int i = 0; i < data->Size; i++)
        data->Out[3][i] = data->Cv[3][i];

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

    master.begin(1000000);
    //HandleAudio = HandleAudioFunction;
}

PerfTimer pt;
PeriodicExecution execPrint(1000);
PeriodicExecution execPrintFast(100);
PeriodicExecution updateState(1);
PeriodicExecution updateMenu(10);

void loop()
{
    pt.Start();
    if (execPrint.Go())
    {
        if (audio.BufferUnderrun)
        {
            LogInfo("Buffer Underrun!!");
            audio.BufferUnderrun = false;
        }

        LogInfof("CPU load: %.2f%%", Timers::GetCpuLoad()*100);

        LogInfof("time : %f", pt.Period());
        LogInfof("time avg: %f", pt.PeriodAvg());
        LogInfof("time max: %f", pt.PeriodMax());
    }

    if (audio.Available())
    {
        auto buf = audio.BeginAudioProcessing();
        HandleAudio(buf);
        audio.EndAudioProcessing();

        controls.UpdatePotState(0);
        controls.UpdatePotState(1);
        menuManager.HandlePotUpdate(0, controls.GetPot(0).Value);
        menuManager.HandlePotUpdate(1, controls.GetPot(1).Value);

        menuManager.Render();
    }

    if (master.finished())
    {
        menuManager.Transfer();
    }
    
    pt.Stop();
}
