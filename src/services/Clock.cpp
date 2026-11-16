#include "Clock.hpp"
#include <string>
#include <ctime>

std::string timeState = "disabled";
time_t timestampa;

std::string Stopwatch()
{
    if (timeState == "active")
    {
        timeState = "disabled";

        time_t timestampb;
        time(&timestampb);

        time_t enttime = timestampb - timestampa;

        return std::to_string(enttime) + " seconds";
    }
    else
    {
        timeState = "active";
        time(&timestampa);

        return "timer started";
    }
}