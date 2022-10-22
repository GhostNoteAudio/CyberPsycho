#pragma once
#include <Arduino.h>
#include "io_buffer.h"

namespace Cyber
{
    class AudioIo
    {
    private:
        uint8_t AdcTxBuf[8][2];
        uint8_t AdcRxBuf[8][2];
        uint8_t DacTxBuf[4][2];
        uint8_t DacRxBuf[2]; // discard
        uint16_t AdcValues[8] = {0};

    public:
        AudioIo();
        void Init();
        void SampleAdc(int channel);
        void ProcessAdcValues();
        void SetDac(int channel, uint16_t value);
        void LatchDac();
        void StartProcessing();
        void StopProcessing();
        void ProcessAudioX();
        bool Available();
        DataBuffer* BeginAudioProcessing();
        void EndAudioProcessing();

    public:
        IntervalTimer ioLoop;
        DataBuffer BufferA;
        DataBuffer BufferB;
        volatile int bufferIdx;
        volatile DataBuffer* BufTransmitting;
        volatile DataBuffer* BufProcessing;
        volatile bool CallbackComplete;

    public:
        volatile bool BufferUnderrun;
    };

    extern AudioIo audio;
    void InvokeProcessAudio();
}
