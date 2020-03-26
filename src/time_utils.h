#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <Arduino.h>

enum class DeltaTimePrecision: uint8_t
{
    DAYS,
    HOURS,
    MINUTES,
    SECONDS
};

String formatDeltaTime(time_t delta, DeltaTimePrecision format);

#endif