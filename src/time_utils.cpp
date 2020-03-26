#include "time_utils.h"

String formatDeltaTime(time_t delta, DeltaTimePrecision format)
{
    delta = labs(delta);

    int seconds = delta % 60;
    delta = delta / 60;

    int minutes = delta % 60;
    delta = delta / 60;

    int hours = delta % 24;
    delta = delta / 24;

    int days = delta;

    char msg[32];

    switch (format)
    {
        case DeltaTimePrecision::DAYS:
            return String(days) + "d";
        case DeltaTimePrecision::HOURS:
            snprintf(msg, sizeof(msg),  "%dd-%02dh", days, hours);
            return msg;
        case DeltaTimePrecision::MINUTES:
            snprintf(msg, sizeof(msg),  "%dd-%02dh-%02dm", days, hours, minutes);
            return msg;
        case DeltaTimePrecision::SECONDS:
            snprintf(msg, sizeof(msg),  "%dd-%02dh-%02dm-%02ds", days, hours, minutes, seconds);
            return msg;
        default:
            ;            
    }
    return "???";
}
