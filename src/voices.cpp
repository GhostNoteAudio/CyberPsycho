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
        float temp1[BUFFER_SIZE];
        float temp2[BUFFER_SIZE];

        GeneratorArgs args;
        args.Bpm = 120;
        args.Gate = fpData->Gate[3];
        
        args.InputLeft = fpData->Mod[3];
        args.OutputLeft = temp1;
        Gen->Process(args);

        args.InputLeft = temp1;
        args.OutputLeft = temp2;
        Inserts[0]->Process(args);

        args.InputLeft = temp2;
        args.OutputLeft = temp1;
        Inserts[1]->Process(args);

        args.InputLeft = temp1;
        args.OutputLeft = temp2;
        Inserts[2]->Process(args);

        args.InputLeft = temp2;
        args.OutputLeft = fpData->Out[3];
        Inserts[3]->Process(args);
    }
}

