#include "Arduino.h"
#include "display.h"
#include "menu.h"
#include "buttons.h"
#include "audio_io.h"
#include "counter.h"

Inputs ins;
AudioIo audio;
Menu m;
DisplayManager display;
ButtonManager buttons;
IntervalTimer audioLoop;

void processAudio();

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

void setup()
{
    BuildMenu();
    //pinMode(LED_BUILTIN,OUTPUT);
    display.Init();

    audio.Init();
    //SPI.begin();
    //SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    
    audioLoop.priority(2);
    audioLoop.begin(processAudio, 22.675737);
}

float yieldTime = 0;
float processingTime = 0;

void processAudio()
{
    int a = micros();
    // wait for value from previous iteration to be complete. Hopefully should never block
    TsyDMASPI0.yield();
    int b = micros();

    // Emit the DAC values that were previously sent to the output
    audio.LatchDac();
    // Process the newly received ADC values
    audio.ProcessAdcValues();

    audio.SampleAdc(3);
    audio.SampleAdc(7);
    audio.SetDac(0, audio.AdcValues[0]);
    audio.SetDac(1, audio.AdcValues[1]);
    //audio.SetDac(2, audio.AdcValues[2]);
    audio.SetDac(3, audio.AdcValues[3]);

    int c = micros();

    yieldTime = yieldTime * 0.99 + (b-a) * 0.01;
    processingTime = processingTime * 0.99 + (c-b) * 0.01;
}

void loop()
{
    Serial.print("Yield time: ");
    Serial.print(yieldTime);
    Serial.print(" Processing time: ");
    Serial.println(processingTime);

    delay(1000);

    // m.Render(display.GetDisplay());
    // display.Transfer();
    // delay(1000);
    // m.MoveDownPage();
    //Serial.print("Adc Val: ");
    //Serial.println(audio.AdcValues[7]);
}
