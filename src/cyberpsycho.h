#include "Arduino.h"
#include "menu_manager.h"
#include "controls.h"
#include "audio_io.h"
#include "input_processor.h"
#include <i2c_driver_wire.h>
#include "audio_yield.h"

#include "periodic_execution.h"
#include "menu.h"
#include "counter.h"
#include "logging.h"
#include "utils.h"
#include "timers.h"

namespace Cyber
{
    extern AudioIo audio;
    extern InputProcessor inProcessor;
    extern MenuManager menuManager;
    extern ControlManager controls;
    extern I2CMaster& i2cMaster;
}