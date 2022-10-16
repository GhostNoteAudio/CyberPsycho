#include "cyberpsycho.h"

// Definitions for singleton objects.

namespace Cyber
{
    AudioIo audio;
    InputProcessor inProcessor;
    DisplayManager displayManager;
    ControlManager controls;
    I2CMaster& i2cMaster = Master;
}
