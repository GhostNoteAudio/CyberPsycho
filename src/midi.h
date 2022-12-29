#pragma once
#include <functional>
#include <Arduino.h>

namespace Cyber
{
    struct MidiMessage
    {
        uint8_t channel;
        uint8_t msgType;
        uint8_t data1;
        uint8_t data2;
    };

    enum class ModWheelMode
    {
        Off = 0,
        Unipolar,
        Bipolar
    };
    
    class Midi
    {
        uint8_t CurrentMessageType = 0;
        int ExpectedByteCount = 0;
        int BufferIdx = 0;
        uint8_t DataBuffer[256] = {0};

        MidiMessage MessageQueue[8];
        int QueueDepth = 0;

        void QueueMessage(MidiMessage msg)
        {
            if (QueueDepth >= 8)
                PopMidiMessage();

            MessageQueue[QueueDepth] = msg;
            QueueDepth++;
        }

    public:
        const int NOTE_OFF = 0x80;
        const int NOTE_ON = 0x90;
        const int POLY_AFT = 0xA0;
        const int CC = 0xB0;
        const int PROGRAM = 0xC0;
        const int CH_AFT = 0xD0;
        const int PITCHBEND = 0xE0;
        const int SYSEX_START = 0xF0;
        const int SYSEX_END = 0xF7;
        const int TIMECODE = 0xF8;
        const float PitchbendScaleInv = 1.0 / 16384.0;

        std::function<void()> HandleMidiClock = []() {};

        bool MidiEnabled = true;
        ModWheelMode Modwheel = ModWheelMode::Off;
        int PitchbendRange = 2;

        uint8_t MidiNote[4] = {0};
        float MidiPitchbend[4] = {0};
        bool MidiGate[4] = {false};

        inline bool Available()
        {
            return QueueDepth > 0;
        }

        inline void HandleMidiMessage(MidiMessage msg)
        {
            if (msg.msgType == NOTE_ON)
            {
                MidiNote[msg.channel] = msg.data1;
                MidiGate[msg.channel] = true;
            }
            else if (msg.msgType == NOTE_OFF)
            {
                if (msg.data1 == MidiNote[msg.channel])
                    MidiGate[msg.channel] = false;
            }
            else if (msg.msgType == PITCHBEND)
            {
                float value = msg.data1 + (msg.data2 << 7);
                value = (value * PitchbendScaleInv) * 2 - 1;
                MidiPitchbend[msg.channel] = value;
            }
        }

        inline MidiMessage PopMidiMessage()
        {
            if (QueueDepth == 0)
                return MidiMessage();

            MidiMessage msg = MessageQueue[0];
            
            MessageQueue[0] = MessageQueue[1];
            MessageQueue[1] = MessageQueue[2];
            MessageQueue[2] = MessageQueue[3];
            MessageQueue[3] = MessageQueue[4];
            MessageQueue[4] = MessageQueue[5];
            MessageQueue[5] = MessageQueue[6];
            MessageQueue[6] = MessageQueue[7];
            QueueDepth--;
            return msg;
        }

        inline void ProcessMidiSerial()
        {
            if (Serial1.available() < 1)
                return;

            while(true)
            {
                int data = Serial1.read();
                if (data == -1) break;
                uint8_t upper = data & 0xF0;

                if (ExpectedByteCount > 0)
                {
                    DataBuffer[BufferIdx] = data;
                    BufferIdx++;
                    ExpectedByteCount--;
                    if (ExpectedByteCount == 0)
                    {
                        BufferIdx = 0;
                        MidiMessage msg;
                        msg.channel = DataBuffer[0] & 0x0F;
                        msg.msgType = DataBuffer[0] & 0xF0;
                        msg.data1 = DataBuffer[1];
                        msg.data2 = DataBuffer[2];
                        QueueMessage(msg);
                    }
                }
                else if (data == SYSEX_START || data == SYSEX_END)
                {
                    // Todo: sysex handling??
                    BufferIdx = 0;
                    ExpectedByteCount = 0;
                }
                else if (data == TIMECODE)
                {
                    ExpectedByteCount = 0;
                    BufferIdx = 0;
                    HandleMidiClock();
                }
                else if (upper >= 0x80 && upper < 0xF0) // 3 byte message
                {
                    DataBuffer[0] = data;
                    BufferIdx = 1;
                    ExpectedByteCount = 2;
                }
            }
        }
    };

    extern Midi midi;
}
