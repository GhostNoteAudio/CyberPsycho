#pragma once

#include <Arduino.h>
#include <TsyDMASPI.h>
#include "pins.h"
#include "counter.h"

void ProcessAudio();

class Inputs
{
    const float Inv12Bit = 1.0 / 4095.0;
    const int GateThresLow = 10;
    const int GateThresHigh = 245;

public:
    uint16_t InputCv[4];
    uint16_t InputMod[4];
    int InputGate[4];

public:    
    // Processed values; scaled, filtered and offset
    float Cv[4];
    float Mod[4];
    bool Gate[4];

    // Adjustments
    uint16_t OffsetCv[4] = {2048};
    uint16_t OffsetMod[4] = {0};
    float ScaleCv[4];
    float ScaleMod[4];
    int16_t GateSpeed[4] = {64}; // 1 = slowest, 255 = instant change, no filtering

    inline void UpdateCvValue(int idx, uint16_t value)
    {
        InputCv[idx] = value;
        Cv[idx] = (InputCv[idx] - OffsetCv[idx]) * ScaleCv[idx] * Inv12Bit;
    }

    inline void UpdateModValue(int idx, uint16_t value)
    {
        InputMod[idx] = value;
        Mod[idx] = (InputMod[idx] - OffsetMod[idx]) * ScaleMod[idx] * Inv12Bit;
    }

    inline void UpdateGateValue(int idx, uint8_t value)
    {
        InputGate[idx] += (-1+2*value) * GateSpeed[idx];
        if (InputGate[idx] > 255) InputGate[idx] = 255;
        if (InputGate[idx] < 0) InputGate[idx] = 0;
        if (InputGate[idx] < GateThresLow) Gate[idx] = false;
        if (InputGate[idx] > GateThresHigh) Gate[idx] = true;
    }
};

class AudioIo
{
public:
    static AudioIo* Instance;

private:
    uint8_t AdcTxBuf[8][3];
    uint8_t AdcRxBuf[8][3];
    uint8_t DacTxBuf[4][2];
    uint8_t DacRxBuf[2]; // discard

public:
    uint16_t AdcValues[8] = {0};

    inline void Init()
    {
        pinMode(PIN_CS_DAC0, OUTPUT);
        pinMode(PIN_CS_DAC1, OUTPUT);
        pinMode(PIN_CS_ADC, OUTPUT);
        pinMode(PIN_LATCH, OUTPUT);
        digitalWrite(PIN_CS_DAC0, HIGH);
        digitalWrite(PIN_CS_DAC1, HIGH);
        digitalWrite(PIN_CS_ADC, HIGH);
        digitalWrite(PIN_LATCH, HIGH);
        TsyDMASPI0.begin(SPISettings(12000000, MSBFIRST, SPI_MODE0));

        Instance = this;
    }

    inline void SampleAdc(int channel)
    {
        uint8_t* data = AdcTxBuf[channel];
        data[0] = 0b00000110 | (channel & 0b100) >> 2;
        data[1] = (channel & 0b011) << 6;
        data[2] = 0;
        
        TsyDMASPI0.queue(AdcTxBuf[channel], AdcRxBuf[channel], 3, PIN_CS_ADC);
    }

    inline void ProcessAdcValues()
    {
        for (int i = 0; i < 8; i++)
        {
            AdcValues[i] = ((AdcRxBuf[i][1] & 0b00001111) << 8) | AdcRxBuf[i][2];
        }
    }

    inline void SetDac(int channel, uint16_t value)
    {
        if (value > (1u << 12) - 1) 
            value = (1u << 12) - 1;
        
        uint8_t* tx = DacTxBuf[channel];
        uint8_t* rx = DacRxBuf;

        uint8_t chBit = (channel & 0x01) << 7;
        uint8_t csPin = channel < 2 ? PIN_CS_DAC0 : PIN_CS_DAC1;
        tx[0] = chBit | 0x30 | (value >> 8);
        tx[1] = value & 0xFF;

        TsyDMASPI0.queue(tx, rx, 2, csPin);
    }

    inline void LatchDac()
    {
        digitalWrite(PIN_LATCH, LOW);
        SpinWait(20);
        digitalWrite(PIN_LATCH, HIGH);
        SpinWait(20);
    }

    inline void StartProcessing()
    {
        audioLoop.priority(2);
        audioLoop.begin(ProcessAudio, 22.675737);
    }

    float yieldTime = 0;
    float processingTime = 0;

    inline void ProcessAudioX()
    {
        // wait for value from previous iteration to be complete. Hopefully should never block
        TsyDMASPI0.yield();
        // Emit the DAC values that were previously sent to the output
        LatchDac();
        // Process the newly received ADC values
        ProcessAdcValues();

        SampleAdc(3);
        SampleAdc(7);
        SetDac(0, AdcValues[0]);
        SetDac(1, AdcValues[1]);
        //SetDac(2, AdcValues[2]);
        SetDac(3, AdcValues[3]);
    }

private:
    IntervalTimer audioLoop;
};

inline void ProcessAudio()
{
    AudioIo::Instance->ProcessAudioX();
}
