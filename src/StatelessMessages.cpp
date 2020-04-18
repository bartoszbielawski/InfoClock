#include <StatelessMessages.h>
#include <time.h>
#include <utils.h>
#include <time_utils.h>

using MsgFun = String (*)();

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
