#include "voices.h"

namespace Cyber
{
    namespace Voices
    {
        Voice Voices[4];
        uint8_t ActiveVoice = 0;
    }

    void Voice::Process(FpBuffer* fpData)
    {
        float temp[BUFFER_SIZE];
        GeneratorArgs args;
        args.Bpm = 120;
        args.InputLeft = fpData->Mod[3];
        args.OutputLeft = temp;
        args.Gate = fpData->Gate[3];

        Gen->Process(args);

        args.InputLeft = temp;
        args.OutputLeft = fpData->Out[3];

        mmf.Process(args);
    }
}

