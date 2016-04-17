#include "common.h"
#include <sstream>
#include <iterator>
#include <time.h>
#include <sys/time.h>

using namespace std;

string ColorToStr(LedColor color)
{
    switch (color)
    {
        case LED_RED: return "red";
        case LED_GREEN: return "green";
        case LED_BLUE: return "blue";
        default: return "unknown";
    }
}

LedColor StrToColor(string str)
{
    if (str == "red") return LED_RED;
    if (str == "green") return LED_GREEN;
    if (str == "blue") return LED_BLUE;
    return LED_UNKNOWN;
}

void split(const string &str, vector<string> &tokens)
{
    istringstream iss(str);
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter(tokens));
}

bool IsCorrectRate(int rate)
{
    return (rate >= 0) && (rate <= 5);
}

long elapsed_ms(timespec &ts)
{
    timespec tsCurr;
    clock_gettime(CLOCK_MONOTONIC, &tsCurr);

    return (tsCurr.tv_sec - ts.tv_sec)*1000
            + (tsCurr.tv_nsec - ts.tv_nsec) / 1000000;
}
