#include "Arduino.h"
#include "cyberpsycho.h"
#include <SdFat.h>

using namespace Cyber;



void HandleAudioFunction(DataBuffer* data)
{
    Cyber::Scope::ProcessScope(data);
    auto fpData = inProcessor.ConvertToFp(data);
    //auto min = Utils::Min(fpData.Cv[3], fpData.Size);
    //auto max = Utils::Max(fpData.Cv[3], fpData.Size);
    //LogInfof("Min: %f - Max: %f", min, max)

    for (int i = 0; i < data->Size; i++)
        data->Out[3][i] = data->Mod[3][i];

    //delayMicroseconds(600);
}

SdFat sd;

void setup()
{
    pinMode(PIN_GATE0, INPUT);
    pinMode(PIN_GATE1, INPUT);
    pinMode(PIN_GATE2, INPUT);
    pinMode(PIN_GATE3, INPUT);
    pinMode(PIN_CLK, INPUT);

    pinMode(PIN_LATCH_DAC, OUTPUT);
    pinMode(PIN_CS_SD, OUTPUT);
    pinMode(PIN_CS_ADC, OUTPUT);
    pinMode(PIN_CS_DAC1, OUTPUT);
    pinMode(PIN_CS_DAC0, OUTPUT);

    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_MISO, INPUT);
    pinMode(PIN_SCK, OUTPUT);

    pinMode(PIN_LATCH_LED, OUTPUT);
    
    pinMode(PIN_MUX_C, OUTPUT);
    pinMode(PIN_MUX_B, OUTPUT);
    pinMode(PIN_MUX_A, OUTPUT);
    
    pinMode(PIN_ENC_A, INPUT);
    pinMode(PIN_ENC_B, INPUT);
    pinMode(PIN_BTN_IN, INPUT);
    pinMode(PIN_POT_IN, INPUT);

    Serial1.begin(31250); // Midi input
    Serial.begin(9600); // USB Serial
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

    SPI.begin();
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
        for (int i = 0; i < 8; i++)
        {
            YieldAudio();
            controls.UpdatePotAndButton(i);
        }

        controls.UpdateEncoderState();
    }

    if (updateMenu.Go())
    {
        auto ev = controls.GetEncoderDelta();
        auto eb = controls.GetEncoderButton();
        if (ev.IsNew)
            Menus::ActiveMenu->HandleEncoder(ev.Value);
        
        if (eb.IsNew)
            Menus::ActiveMenu->HandleEncoderSwitch(eb.Value);

        for (int i = 0; i < 4; i++)
        {
            auto potVal = controls.GetPot(i);
            if (potVal.IsNew)
            {
                Menus::ActiveMenu->HandlePot(i, potVal.Value);
            }
        }
        
        for (int i = 0; i < 8; i++)
        {
            auto btn = controls.GetButton(i);
            if (btn.IsNew)
            {
                Menus::ActiveMenu->HandleSwitch(i, btn.Value);
            }
        }
        
        YieldAudio();
        displayManager.Render();
    }

    YieldAudio();

    if (i2cMaster.finished())
    {
        displayManager.Transfer();
    }
}
