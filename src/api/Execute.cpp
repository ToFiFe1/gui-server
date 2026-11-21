
#include "Execute.hpp"

#include <api/Apis.hpp>
#include <vulcan/ex.hpp>
#include <services/Clock.hpp>
#include <update/Update.hpp>



#include <cstdlib>
#include <fstream>
#include <string>

int x = 0;
int y = 0;

std::string Execute(const std::string& command)
{
    if (command == "redirect.html")
    {
        return loadHTTPrequest("redirect.html", "200 OK");
    }
   
    else if (command == "add")
    {
        ++x;
        return apiResponse(std::to_string(x));
    }
    else if (command == "sub")
    {
        --x;
        return apiResponse(std::to_string(x));
    }
    else if (command == "getx")
    {
        return apiResponse(std::to_string(x));
    }
    else if (command == "TrigStop")
    {
        return apiResponse(Stopwatch());
    }
    else if (command == "vulkan")
    {
        renderVulkan();
        return apiResponse(R"({"success":true})");
    }
    else if (command == "checkout")
    {
        return apiResponse(std::to_string(x));
    }
    else if (command == "getStatus") {
        return apiResponse("true");
    }
    else if (command == "n8n_1") {
        return handleRequest("http://localhost:5678/webhook/9f8e380c-b540-4957-9de1-1f8f5139d179");
    }
    else if (command == "gemeinde") {
        return loadHTTPrequest("mavuno.html", "200 OK");
       
    }
    else if (command == "Update") {

        Update();
    }
    
    else if (command == "getUIConfig")
    {
        std::ifstream file(
            "modules/ConfigUI/config.json",
            std::ios::binary
        );

        if (!file.is_open())
        {
            return loadHTTPrequest(
                "errors/404.html",
                "404 Not Found"
            );
        }

        std::string config;
        std::string line;

        while (std::getline(file, line))
        {
            config += line;
            config += '\n';
        }

        if (config.empty())
        {
            return loadHTTPrequest(
                "errors/404.html",
                "404 Not Found"
            );
        }

        return apiResponse(config);
    }

    return loadHTTPrequest(
        "errors/404.html",
        "404 Not Found"
    );
}
