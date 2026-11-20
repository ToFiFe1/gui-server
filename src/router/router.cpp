#include "router.hpp"

#include <api/Apis.hpp>

std::string routeRequest(const std::string& path)
{
    if (path == "/")
    {
        return onload();
    }

    return handleRequest(path);
}