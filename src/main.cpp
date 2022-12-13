#include "Arduino.h"
#include "cyberpsycho.h"
#include <SdFat.h>
#include "modules/init.h"
#include "perftest.h"
#include "fast_curves.h"

#include "generators/kick1.h"
#include "generators/superwave.h"
#include "generators/multimodeFilter.h"
#include "generators/redux.h"
#include "generators/basicDrive.h"
#include "generators/eqShelf.h"

using namespace Cyber;

void HandleAudioFunction(DataBuffer* data)
{
    Cyber::Scope::ProcessScope(data);
    auto fpData = inProcessor.ConvertToFp(data);
    //Voices::GetActiveVoice()->Process(fpData);
    //delayMicroseconds(100);
    //Utils::To12Bit(data->Out[0], fpData->Out[0], data->Size);
    //Utils::To12Bit(data->Out[1], fpData->Out[1], data->Size);
    //Utils::To12Bit(data->Out[2], fpData->Out[2], data->Size);
    //Utils::To12Bit(data->Out[3], fpData->Out[3], data->Size);
}

SdFat sd;

void PreventStartupBleep()
{
    // Primes the serial port for data, otherwise there's a ~100ms stall just after startup
    for (int i = 0; i < 10; i++)
    {
        delay(10);
        LogInfo("Starting up...");
    }
}

void RegisterAllGenerators()
{
    generatorRegistry.Add<Kick1>();
    generatorRegistry.Add<Superwave>();
    generatorRegistry.Add<MultimodeFilter>();
    generatorRegistry.Add<Redux>();
    generatorRegistry.Add<BasicDrive>();
    generatorRegistry.Add<EQShelf>();
    generatorRegistry.Add<Bypass>();

    Voices::InitVoices();
    Voices::Voices[0].Gen = generatorRegistry.CreateInstanceById("GNA-Superwave");
}

void setup()
{
    Modules::Init();
    Utils::Init();
    FastCurves::Init();

    // Midi input
    // Note that Tx1 Send is actually PIN_GATE0, but we never send serial data, only use RX
    Serial1.begin(31250); 

    // USB Serial. Read baudrate is much, much higher
    Serial.begin(9600);
    //while(!Serial) {}

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

    Serial.println("Starting...");

    Menus::Init();
    displayManager.Init();
    Menus::ActiveMenu = &Menus::initMenu;

    audio.Init();
    Serial.println("Done");

    i2cMaster.begin(1000000);
    HandleAudioCb = HandleAudioFunction;

    PreventStartupBleep();
    RegisterAllGenerators();
    SPI.begin();
    audio.StartProcessing();
}

PeriodicExecution execPrint(1000);
PeriodicExecution execPrintFast(100);
PeriodicExecution updateState(1);
PeriodicExecution updateMenu(10);

void loop()
{

    //RunBenchmark();
    
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
        float cpuLoad = GetCpuLoad();
        LogInfof("Audio Time : %f %f %f", pa->Period(), pa->PeriodAvg(), pa->PeriodDecay());
        LogInfof("Yield Time : %f %f %f", py->Period(), py->PeriodAvg(), py->PeriodDecay());
        LogInfof("IO Time : %f %f %f", pi->Period(), pi->PeriodAvg(), pi->PeriodDecay());
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
