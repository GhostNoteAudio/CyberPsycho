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

InputProcessor inProcessor;
AudioIo audio;
Menu m;
MenuManager menuManager;
ControlManager controls;

I2CMaster& master = Master;

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

    Wire.setClock(1000000);
}

void HandleAudio(DataBuffer* data)
{
    auto fpData = inProcessor.ConvertToFp(data);
    //auto min = Utils::Min(fpData.Cv[3], fpData.Size);
    //auto max = Utils::Max(fpData.Cv[3], fpData.Size);
    //LogInfof("Min: %f - Max: %f", min, max)

    for (int i = 0; i < 16; i++)
        data->Out[3][i] = data->Cv[3][i];

    //delayMicroseconds(340);
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
}

int tsLast = 0;
int t = 0;
const int chunkSize = 16;
int totalChunks = 1024 / chunkSize;

uint8_t txbufStart[7];
uint8_t txbuf[1 + chunkSize];

void transmitDisplay()
{
    auto buffer = menuManager.GetDisplay()->getBuffer();
    uint8_t *ptr = &buffer[t*chunkSize];
    
    if (t == -1)
    {
        txbufStart[0] = 0x00;
        txbufStart[1] = 0x22; // set page address
        txbufStart[2] = 0x00; // first page
        txbufStart[3] = 0xFF; // last page
        txbufStart[4] = 0x21; // set column start address
        txbufStart[5] = 0x00; // first column
        txbufStart[6] = 127; // last column
        master.write_async(0x3C, txbufStart, 7, true);
        t = 0;
        return;
    }

    txbuf[0] = (uint8_t)0x40;
    for (int i = 0; i < chunkSize; i++)
    {
        txbuf[i+1] = ptr[i];
    }
    
    master.write_async(0x3C, txbuf, 1+chunkSize, true);

    t++;
    if (t == totalChunks)
        t = -1;
}

PerfTimer pt;

void loop()
{
    pt.Start();
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

        LogInfof("time : %f", pt.Period());
        LogInfof("time avg: %f", pt.PeriodAvg());
        LogInfof("time max: %f", pt.PeriodMax());
    }

    
    
    if (audio.Available())
    {
        auto buf = audio.BeginAudioCallback();
        HandleAudio(buf);
        audio.EndAudioCallback();

        controls.UpdatePotState(0);
        menuManager.HandlePotUpdate(0, controls.GetPot(0));
        menuManager.Render();
    }

    if (master.finished())
    {
        transmitDisplay();
    }
    
    pt.Stop();
}