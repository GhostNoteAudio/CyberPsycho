#include "logging.h"

int LogLevel = LogLevelInfo;


void ts()
{
    // prints a timestamp to the log - useful for timing-based debugging
    auto time = micros();
    auto seconds = (int)(time / 1000000);
    auto millis = (int)(time / 1000) - seconds * 1000;
    auto micro = time - seconds * 1000000 - millis * 1000;
    seconds = seconds % 10000; // overflow seconds after 10k seconds (~2.7 hours)
    Serial.printf("[%04d:%03d.%03d] ", seconds, millis, micro);
}