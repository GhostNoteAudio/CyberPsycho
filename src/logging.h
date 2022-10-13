#pragma once
#include "Arduino.h"

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

#define LogDebug(msg) { if (LogLevel >= LogLevelDebug) { ts(); Serial.println(msg); }}
#define LogDebugf(fmt, ...) {if (LogLevel >= LogLevelDebug) { ts(); Serial.printf(fmt, __VA_ARGS__); Serial.println(""); }}
#define LogInfo(msg) {if (LogLevel >= LogLevelInfo) { ts(); Serial.println(msg); }}
#define LogInfof(fmt, ...) {if (LogLevel >= LogLevelInfo) { ts(); Serial.printf(fmt, __VA_ARGS__); Serial.println(""); }}
#define LogWarn(msg) {if (LogLevel >= LogLevelWarn) { ts(); Serial.println(msg); }}
#define LogWarnf(fmt, ...) {if (LogLevel >= LogLevelWarn) { ts(); Serial.printf(fmt, __VA_ARGS__); Serial.println(""); }}
#define LogError(msg) {if (LogLevel >= LogLevelError) { ts(); Serial.println(msg); }}
#define LogErrorf(fmt, ...) {if (LogLevel >= LogLevelError) { ts(); Serial.printf(fmt, __VA_ARGS__); Serial.println(""); }}
