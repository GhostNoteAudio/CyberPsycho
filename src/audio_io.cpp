#include "audio_io.h"
#include <Arduino.h>
#include <TsyDMASPI.h>
#include <functional>

#include "pins.h"
#include "counter.h"
#include "io_buffer.h"
#include "timers.h"
#include "constants.h"
#include "logging.h"

namespace Cyber
{
    AudioIo::AudioIo()
    {
        bufferIdx = 0;
        BufTransmitting = &BufferA;
        BufProcessing = &BufferB;
        CallbackComplete = true;
        BufferUnderrun = false;
    }

    void AudioIo::Init()
    {        
        pinMode(PIN_CS_DAC0, OUTPUT);
        pinMode(PIN_CS_DAC1, OUTPUT);
        pinMode(PIN_CS_ADC, OUTPUT);
        pinMode(PIN_LATCH_DAC, OUTPUT);
        digitalWrite(PIN_CS_DAC0, HIGH);
        digitalWrite(PIN_CS_DAC1, HIGH);
        digitalWrite(PIN_CS_ADC, HIGH);
        digitalWrite(PIN_LATCH_DAC, HIGH);
        TsyDMASPI0.begin(SPISettings(12000000, MSBFIRST, SPI_MODE0));
    }

    void AudioIo::SampleAdc(int channel)
    {
        uint8_t* data = AdcTxBuf[channel];
        data[0] = channel << 3;
        data[1] = 0;
        TsyDMASPI0.queue(AdcTxBuf[channel], AdcRxBuf[channel], 2, PIN_CS_ADC);
    }

    void AudioIo::ProcessAdcValues()
    {
        for (int i = 0; i < 8; i++)
        {
            AdcValues[i] = ((AdcRxBuf[i][0] & 0x0F) << 8) | AdcRxBuf[i][1];
        }
    }

    void AudioIo::SetDac(int channel, uint16_t value)
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

    void AudioIo::LatchDac()
    {
        digitalWrite(PIN_LATCH_DAC, LOW);
        SpinWait(20);
        digitalWrite(PIN_LATCH_DAC, HIGH);
        SpinWait(20);
    }

    void AudioIo::StartProcessing()
    {
        BufTransmitting = &BufferA;
        BufProcessing = &BufferB;
        CallbackComplete = true;
        BufferUnderrun = false;
        bufferIdx = 0;
        ioLoop.begin(InvokeProcessAudio, 1000000.0/SAMPLERATE);
    }

    void AudioIo::StopProcessing()
    {
        ioLoop.end();
    }

    void AudioIo::ProcessAudioX()
    {
        GetPerfIo()->Start();
        
        if (TsyDMASPI0.remained() > 0)
        {
            BufferUnderrun = true;
            return;
        }

        // wait for value from previous iteration to be complete. Hopefully should never block
        TsyDMASPI0.yield();
        // Emit the DAC values that were previously sent to the output
        LatchDac();
        // Process the newly received ADC values
        ProcessAdcValues();

        BufTransmitting->Cv[0][bufferIdx] = AdcValues[0];
        BufTransmitting->Cv[1][bufferIdx] = AdcValues[1];
        BufTransmitting->Cv[2][bufferIdx] = AdcValues[2];
        BufTransmitting->Cv[3][bufferIdx] = AdcValues[3];
        BufTransmitting->Mod[0][bufferIdx] = AdcValues[4];
        BufTransmitting->Mod[1][bufferIdx] = AdcValues[5];
        BufTransmitting->Mod[2][bufferIdx] = AdcValues[6];
        BufTransmitting->Mod[3][bufferIdx] = AdcValues[7];
        BufTransmitting->Gate[0][bufferIdx] = digitalReadFast(PIN_GATE0);
        BufTransmitting->Gate[1][bufferIdx] = digitalReadFast(PIN_GATE1);
        BufTransmitting->Gate[2][bufferIdx] = digitalReadFast(PIN_GATE2);
        BufTransmitting->Gate[3][bufferIdx] = digitalReadFast(PIN_GATE3);
        
        SampleAdc(3);
        SampleAdc(4);
        //SampleAdc(5);
        SampleAdc(6);
        SampleAdc(7);
        //SetDac(0, BufTransmitting->Out[0][bufferIdx]);
        //SetDac(1, BufTransmitting->Out[1][bufferIdx]);
        SetDac(2, BufTransmitting->Out[2][bufferIdx]);
        SetDac(3, BufTransmitting->Out[3][bufferIdx]);

        bufferIdx++;
        if (bufferIdx == BUFFER_SIZE)
        {
            if (!CallbackComplete)
            {
                BufferUnderrun = true;
            }

            bufferIdx = 0;
            auto tmp = BufTransmitting;
            BufTransmitting = BufProcessing;
            BufProcessing = tmp;
            CallbackComplete = false;
        }

        GetPerfIo()->Stop();
    }

    bool AudioIo::Available()
    {
        return !CallbackComplete;
    }

    DataBuffer* AudioIo::BeginAudioProcessing()
    {
        GetPerfAudio()->Start();
        return const_cast<DataBuffer*>(BufProcessing);
    }

    void AudioIo::EndAudioProcessing()
    {
        CallbackComplete = true;
        GetPerfAudio()->Stop();
    }


    void InvokeProcessAudio()
    {
        audio.ProcessAudioX();
    }
}
