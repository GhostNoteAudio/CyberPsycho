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
#include "tempo.h"

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
        digitalWrite(PIN_CS_DAC0, HIGH);
        digitalWrite(PIN_CS_DAC1, HIGH);
        digitalWrite(PIN_CS_ADC, HIGH);
        digitalWrite(PIN_CS_SD, HIGH);
        digitalWrite(PIN_LATCH_DAC, HIGH);
        TsyDMASPI0.csPins[0] = PIN_CS_DAC0;
        TsyDMASPI0.csPins[1] = PIN_CS_DAC1;
        TsyDMASPI0.csPins[2] = PIN_CS_ADC;
        TsyDMASPI0.csPins[3] = PIN_CS_SD;
        TsyDMASPI0.begin(SPISettings(24000000, MSBFIRST, SPI_MODE0));
        PrepAdcBuffer();
    }

    void AudioIo::PrepAdcBuffer()
    {
        // during each cycle, we send the channel adddress for the NEXT conversion.
        // This does mean channel 0 lags by one sample compared to the others, but shouldn't be an issue
        // to prevent lag, could send a "primer" message and set the next read channel to 0, but that requires 2 extra bytes transmitted
        for (int i = 0; i < 8; i++)
        {
            AdcTxBuf[2*i+0] = ((i+1) << 3) & 0b00111000;
            AdcTxBuf[2*i+1] = 0;
        }
    }

    void AudioIo::SampleAdc()
    {
        //LogInfo("Queueing adc sampling");
        //PrepAdcBuffer();
        TsyDMASPI0.queue(AdcTxBuf, AdcRxBuf, 16, PIN_CS_ADC);
    }

    void AudioIo::ProcessAdcValues()
    {
        for (int i = 0; i < 8; i++)
        {
            uint8_t byte0 = AdcRxBuf[2*i+0]; 
            uint8_t byte1 = AdcRxBuf[2*i+1];
            AdcValues[i] = ((byte0 & 0x0F) << 8) | byte1;
        }
    }

    void AudioIo::SetDac(int channel, uint16_t value)
    {
        if (value > 4095) 
            value = 4095;
        
        uint8_t* tx = DacTxBuf[channel];
        uint8_t* rx = DacRxBuf;

        uint8_t chBit = (channel & 0x01) << 7;
        uint8_t csPin = channel < 2 ? PIN_CS_DAC0 : PIN_CS_DAC1;
        tx[0] = chBit | 0x30 | (value >> 8);
        tx[1] = value & 0xFF;

        TsyDMASPI0.queue(tx, rx, 2, csPin);
    }

    void AudioIo::PushLeds()
    {
        TsyDMASPI0.queue(&LedTx, &LedRx, 1, 255);
    }

    uint8_t AudioIo::GetLed()
    {
        return LedTx;
    }

    void AudioIo::SetLed(uint8_t mask)
    {
        LedTx = mask;
    }

    void AudioIo::StartProcessing()
    {
        BufTransmitting = &BufferA;
        BufProcessing = &BufferB;
        CallbackComplete = true;
        BufferUnderrun = false;
        bufferIdx = 0;
        ioLoop.begin(InvokeProcessAudio, 1000000.0/SAMPLERATE);
        //ioLoop.begin(InvokeProcessAudio, 1000000);
        Enabled = true;
    }

    void AudioIo::StopProcessing()
    {
        ioLoop.end();
        Enabled = false;
    }

    bool AudioIo::IsProcessingEnabled()
    {
        return Enabled;
    }

    void AudioIo::ProcessAudioX()
    {
        GetPerfIo()->Start();
        
        if (TsyDMASPI0.remained() > 0)
        {
            BufferUnderrun = true;
            return;
        }

        // Emit the DAC values that were previously sent to the output
        //LatchDac();
        digitalWriteFast(PIN_LATCH_DAC, LOW);
        digitalWriteFast(PIN_LATCH_LED, HIGH);
        // Process the newly received ADC values
        ProcessAdcValues();
        digitalWriteFast(PIN_LATCH_DAC, HIGH);
        digitalWriteFast(PIN_LATCH_LED, LOW);

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
        
        SetDac(0, BufTransmitting->Out[0][bufferIdx]);
        SetDac(1, BufTransmitting->Out[1][bufferIdx]);
        SetDac(2, BufTransmitting->Out[2][bufferIdx]);
        SetDac(3, BufTransmitting->Out[3][bufferIdx]);
        SampleAdc();

        auto clk = digitalReadFast(PIN_CLK);
        tempoState.TickClk(clk);

        PushLeds();

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

        TsyDMASPI0.beginTransaction();
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
