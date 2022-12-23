#include "Arduino.h"
#include "cyberpsycho.h"
#include <SdFat.h>
#include "modules/init.h"
#include "perftest.h"
#include "fast_curves.h"

#include "generators/superwave.h"
#include "generators/quad.h"

#include "slots/A440.h"
#include "slots/Noise.h"
#include "slots/Blank.h"
#include "slots/Kick1.h"
#include "slots/Drom.h"
#include "slots/BasicDrive.h"
#include "slots/EqShelf.h"

using namespace Cyber;

void HandleAudioFunction(DataBuffer* data)
{
    Cyber::Scope::ProcessScope(data);
    voice.Process(data);
}

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
    generatorRegistry.AddGen<Superwave>();
    generatorRegistry.AddGen<Quad>();

    generatorRegistry.AddSlotGen<Blank>();
    generatorRegistry.AddSlotGen<A440Hz>();
    generatorRegistry.AddSlotGen<Noise>();
    generatorRegistry.AddSlotGen<Kick1>();
    generatorRegistry.AddSlotGen<DRom>();
    generatorRegistry.AddSlotGen<BasicDrive>();
    generatorRegistry.AddSlotGen<EqShelf>();
    // generatorRegistry.Add<MultimodeFilter>();
    // generatorRegistry.Add<Redux>();
    // generatorRegistry.Add<Bypass>();

    voice.Init();
    voice.SetGenerator(generatorRegistry.GetGenIndexById("GNA-Quad"));
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

    digitalWrite(PIN_MUX_A, LOW);
    digitalWrite(PIN_MUX_B, LOW);
    digitalWrite(PIN_MUX_C, LOW);

    digitalWrite(PIN_CS_ADC, HIGH);
    digitalWrite(PIN_CS_DAC0, HIGH);
    digitalWrite(PIN_CS_DAC1, HIGH);
    digitalWrite(PIN_CS_SD, HIGH);

    Storage::InitStorage();
    SPI.begin();
    displayManager.Init();

    audio.Init();
    Serial.println("Done");

    i2cMaster.begin(1000000);
    HandleAudioCb = HandleAudioFunction;

    PreventStartupBleep();
    RegisterAllGenerators();
    Menus::Init();
    Storage::LoadGlobalState();

    Menus::calibrateMenu.ReapplyAllValues();
    Menus::globalMenu.ReapplyAllValues();
    Menus::pitchTrigMenu.ReapplyAllValues();
    voice.Gen->GetMenu()->ReapplyAllValues();

    audio.StartProcessing();
}

PeriodicExecution execPrint(1000);
PeriodicExecution execPrintFast(100);
PeriodicExecution updateState(1);
PeriodicExecution updateMenu(30);

void loop()
{
    execPrint.active = false;
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
        controls.UpdatePotAndButton();
        controls.UpdateEncoderState();

        for (int i = 0; i < 8; i++)
        {
            auto btn = controls.GetButton(i);
            if (btn.IsNew)
            {
                LogInfof("Button: %d, state: %d", i, btn.Value);
                displayManager.ActiveMenu->HandleSwitch(i, btn.Value);
            }
        }

        YieldAudio();
    }

    if (updateMenu.Go())
    {
        auto ev = controls.GetEncoderDelta();
        auto eb = controls.GetEncoderButton();
        if (ev.IsNew)
            displayManager.ActiveMenu->HandleEncoder(ev.Value);
        
        if (eb.IsNew)
            displayManager.ActiveMenu->HandleEncoderSwitch(eb.Value);

        for (int i = 0; i < 4; i++)
        {
            auto potVal = controls.GetPot(i);
            if (potVal.IsNew)
            {
                displayManager.ActiveMenu->HandlePot(i, potVal.Value);
            }
        }
        
        YieldAudio();
        displayManager.Render();
        YieldAudio();
    }

    if (i2cMaster.finished())
    {
        displayManager.Transfer();
    }
}
