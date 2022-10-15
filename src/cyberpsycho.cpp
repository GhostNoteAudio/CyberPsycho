#include "cyberpsycho.h"

// Definitions for singleton objects.

namespace Cyber
{
    AudioIo audio;
    InputProcessor inProcessor;
    MenuManager menuManager;
    ControlManager controls;
    I2CMaster& i2cMaster = Master;
}
