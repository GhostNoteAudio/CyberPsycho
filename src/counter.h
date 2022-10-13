#pragma once
#include <Arduino.h>

uint64_t GetCounter();
void SpinWait(uint64_t cycles);
