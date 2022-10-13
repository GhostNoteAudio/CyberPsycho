#include "Arduino.h"
#include "display.h"
#include "menu.h"
#include "controls.h"
#include "audio_io.h"
#include "counter.h"
#include "logging.h"
Inputs ins;
AudioIo audio;
Menu m;
DisplayManager display;
ControlManager buttons;

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

    m.Length = 7;
    m.SelectedItem = 3;
    m.TopItem = 0;
    m.EnableSelection = true;
    m.QuadMode = true;
}

void HandleAudio(IOBuffer<16>* data)
{
    for (int i = 0; i < 16; i++)
        data->Out[3][i] = data->Cv[3][i];

    delayMicroseconds(340);
}


void setup()
{
    while(!Serial) {}
    Serial.println("Starting...");

    BuildMenu();
    //pinMode(LED_BUILTIN,OUTPUT);
    display.Init();

    audio.Init();
    audio.StartProcessing();
}

int tsLast = 0;

void loop()
{
    auto tsx = micros();

    if (tsx-tsLast > 1000000)
    {
        if (audio.BufferUnderrun)
        {
            LogInfo("Buffer Underrun!!");
            audio.BufferUnderrun = false;
        }

        LogInfof("CPU load: %.2f%%", Timers::GetCpuLoad()*100);
        tsLast = tsx;
    }
    
    if (audio.Available())
    {
        auto buf = audio.BeginAudioCallback();
        HandleAudio(buf);
        audio.EndAudioCallback();
    }

    
    // m.Render(display.GetDisplay());
    // display.Transfer();
    // delay(1000);
    // m.MoveDownPage();
    //Serial.print("Adc Val: ");
    //Serial.println(audio.AdcValues[7]);
}
