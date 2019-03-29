#include <BrexitCountdownDisplay.h>
#include <time.h>
#include <utils.h>

const static time_t BREXIT_DATE = 1553904000 + (14 * 24 * 3600);

String getBrexitDowncountMessage()
{
    time_t now = time(nullptr);
    logPrintfX("BDC", "Now: %d", now);
    if (now < 1000000000)
        return String();        //empty string if time is now known

    time_t diff = BREXIT_DATE - now;

    logPrintfX(F("BDC"), F("Time diff: %d"), diff);

    if (diff < 0)
    {
        return "Bye UK :)";
    }

    int seconds = diff % 60;
    diff = diff / 60;

    int minutes = diff % 60;
    diff = diff / 60;

    int hours = diff % 24;
    diff = diff / 24;

    int days = diff;

    char buffer[128];

    snprintf(buffer, sizeof(buffer), "Brexit (probably, make up your mind, will you?) in %dd-%02dh-%02dm-%02ds", days, hours, minutes, seconds);

    logPrintfX(F("BDC"), F("%s"), buffer);

    return String(buffer);       
}


