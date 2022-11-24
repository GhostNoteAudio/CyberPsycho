#pragma once

namespace Cyber
{
    enum class ModSource
    {
        Mod1 = 0,
        Mod2,
        Mod3,
        Mod4,

        Cv1,
        Cv2,
        Cv3,
        Cv4,

        Gate1,
        Gate2,
        Gate3,
        Gate4,

        Env1,
        Env2,
        Lfo1,
        Lfo2
    };

    enum class ModDest
    {
        Voice = 0,
        Generator,
        Insert1,
        Insert2,
        Insert3,
        Insert4,
        Env1,
        Env2,
        Lfo1,
        Lfo2,
        ModMatrix,
    };
}
