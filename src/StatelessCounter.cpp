#include <StatelessCounter.h>
#include <time.h>
#include <utils.h>

using MsgFun = String (*)();

enum class DeltaTimePrecision
{
    DAYS,
    HOURS,
    MINUTES,
    SECONDS
};

static String formatDeltaTime(time_t delta, DeltaTimePrecision format)
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


String getQuarantineCounterMessage()
{
    const static time_t when = 1584442800 + 3600; //Tue, 17 mar 2020 @ 12:00 CET
    time_t delta = time(NULL) - when;

    if (delta < 0)
        return "...";

    return "In lockdown for " + formatDeltaTime(delta, DeltaTimePrecision::HOURS);
}

static const MsgFun msg_funs[] = {getQuarantineCounterMessage};
static int msg_fun_index = -1;
static int msg_fun_count = sizeof(msg_funs) / sizeof(msg_funs[0]);

String getMessage()
{
    msg_fun_index++;
    if (msg_fun_index >= msg_fun_count)
        msg_fun_index = 0;
    return msg_funs[msg_fun_index]();
}


