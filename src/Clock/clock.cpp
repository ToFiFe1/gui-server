
#include "clock.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

std::string NewClock()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::ostringstream output;
    output << std::put_time(&localTime, "%H:%M:%S");

    return output.str();
}
