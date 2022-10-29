#pragma once
#include "Arduino.h"

namespace Cyber
{
    extern int LogLevel;

    const int LogLevelDebug = 4;
    const int LogLevelInfo = 3;
    const int LogLevelWarn = 2;
    const int LogLevelError = 1;

    inline void SetLogLevel(int newLevel)
    {
        LogLevel = newLevel;
    }

    void ts();
}

#define LogDebug(msg) { if (Cyber::LogLevel >= Cyber::LogLevelDebug) { Cyber::ts(); Serial.println(msg); }}
#define LogDebugf(fmt, ...) {if (Cyber::LogLevel >= Cyber::LogLevelDebug) { Cyber::ts(); Serial.printf(fmt, __VA_ARGS__); Serial.println(""); }}
#define LogInfo(msg) {if (Cyber::LogLevel >= Cyber::LogLevelInfo) { Cyber::ts(); Serial.println(msg); }}
#define LogInfof(fmt, ...) {if (Cyber::LogLevel >= Cyber::LogLevelInfo) { Cyber::ts(); Serial.printf(fmt, __VA_ARGS__); Serial.println(""); }}
#define LogWarn(msg) {if (Cyber::LogLevel >= Cyber::LogLevelWarn) { Cyber::ts(); Serial.println(msg); }}
#define LogWarnf(fmt, ...) {if (Cyber::LogLevel >= Cyber::LogLevelWarn) { Cyber::ts(); Serial.printf(fmt, __VA_ARGS__); Serial.println(""); }}
#define LogError(msg) {if (Cyber::LogLevel >= Cyber::LogLevelError) { Cyber::ts(); Serial.println(msg); }}
#define LogErrorf(fmt, ...) {if (Cyber::LogLevel >= Cyber::LogLevelError) { Cyber::ts(); Serial.printf(fmt, __VA_ARGS__); Serial.println(""); }}
