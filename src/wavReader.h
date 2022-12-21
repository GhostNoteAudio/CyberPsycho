#pragma once
#include <stdint.h>

namespace Cyber
{
    namespace Wav
    {
        struct WaveInfo
        {
            int SampleCount;
            int SampleRate;
            int BytesPerSample;
            int Channels;
            uint8_t* dataPtr;

            inline WaveInfo()
            {
                SampleCount = 0;
                SampleRate = 0;
                BytesPerSample = 0;
                Channels = 0;
                dataPtr = 0;
            }

            inline WaveInfo(int sampleCount, int sampleRate, int bytesPerSample, int channels, uint8_t* data)
            {
                SampleCount = sampleCount;
                SampleRate = sampleRate;
                BytesPerSample = bytesPerSample;
                Channels = channels;
                dataPtr = data;
            }
        };

        inline WaveInfo GetWaveInfo(uint8_t* fileData, int fileLen)
        {
            int* riffPtr = (int*)fileData;
            int* wavePtr = (int*)(&fileData[8]);
            if (*riffPtr != 1179011410) // 'RIFF'
                return WaveInfo();
            if (*wavePtr != 1163280727) // 'WAVE'
                return WaveInfo();

            int* chunk1Id = (int*)(&fileData[12]);
            if (*chunk1Id != 544501094) // 'fmt '
                return WaveInfo();

            //int chunk1Size = *(int*)(&fileData[16]);
            //short audioFormat = *(short*)(&fileData[20]);
            short numChannels = *(short*)(&fileData[22]);
            int sampleRate = *(int*)(&fileData[24]);
            int byteRate = *(int*)(&fileData[28]);

            int index = 36;
            int chunkId, chunkSize;

            while (true)
            {
                if (index >= fileLen)
                    return WaveInfo();

                chunkId = *(int*)(&fileData[index]);
                chunkSize = *(int*)(&fileData[index + 4]);

                if (chunkId == 1635017060) // 'data'
                    break;
                else
                    index += chunkSize + 8;
            }
            
            auto dataPtr = &fileData[index + 8];
            int bytesPerSample = byteRate / sampleRate / numChannels;
            int sampleCount = chunkSize / bytesPerSample / numChannels;
            return WaveInfo(sampleCount, sampleRate, bytesPerSample, numChannels, dataPtr);
        }

        template<typename T>
        inline void GetWaveData(WaveInfo* info, T* target, int channel=0)
        {	
            int stride = info->BytesPerSample * info->Channels;
            float scaler = 1;
            float offset = 0;
            if (info->BytesPerSample == 1)
            {
                scaler = 1.0 / 128.0;
                offset = 128;
            }
            else if (info->BytesPerSample == 2)
                scaler = 1.0 / 32768.0;
            else if (info->BytesPerSample == 3)
                scaler = 1.0 / 16777216.0;
            else if (info->BytesPerSample == 4) // float32 assumed
                scaler = 1.0;

            float postScaler = 1.0;
            if (sizeof(T) == 1) // uint8_t assumed
                postScaler = 256;
            if (sizeof(T) == 2) // int16_t assumed
                postScaler = 32768;

            auto GetValueAt = [&](int idx, int bytes)
            {
                if (bytes == 1)
                    return (float)*(int8_t*)(&info->dataPtr[idx]);
                if (bytes == 2)
                    return (float)*(int16_t*)(&info->dataPtr[idx]);
                if (bytes == 3)
                {
                    uint8_t data[4];
                    data[0] = 0;
                    data[1] = info->dataPtr[idx+0];
                    data[2] = info->dataPtr[idx+1];
                    data[3] = info->dataPtr[idx+2];
                    uint8_t* ptr = &data[0];
                    int* iptr = (int*)ptr;
                    int value = *iptr;
                    return (float)value;
                }
                if (bytes == 4)
                    return *(float*)(&info->dataPtr[idx]);
                return 0.0f;
            };

            // if a non-existent channel is requested, return zeros
            if (channel >= info->Channels)
            {
                for (int i = 0; i < info->SampleCount; i++)
                    target[i] = 0;
                return;
            }

            for (int i = 0; i < info->SampleCount; i++)
            {		
                int idx = i * stride + channel * info->BytesPerSample;
                float value = (GetValueAt(idx, info->BytesPerSample) - offset) * scaler;
                target[i] = (T)(value * postScaler);
            }

        }
    }
}
