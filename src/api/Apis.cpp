#include "Apis.hpp"
#include <iostream>
#include <files/file_manager.hpp>
#include <string>
#include <sstream>
#include <api/Execute.hpp>


std::string response;
std::string file;

static const std::string CORS_HEADERS =
    "Access-Control-Allow-Origin: *\r\n"
    "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
    "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
    "Access-Control-Max-Age: 86400\r\n";

std::string apiResponse(const std::string& body)
{
    return
        "HTTP/1.1 200 OK\r\n"
        + CORS_HEADERS +
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "\r\n" +
        body;
}

std::string loadHTTPrequest(
    const std::string& filename,
    const std::string& status
)
{
    std::string content = readFile(filename);
    std::string filetype = GetFileType(filename);

    std::string response =
        "HTTP/1.1 " + status + "\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Content-Type: " + filetype + "\r\n"
        "Content-Length: "
        + std::to_string(content.size())
        + "\r\n"
        "Connection: close\r\n"
        "\r\n"
        + content;

    return response;
}


std::string handleRequest(const std::string& api)
{
    if (api.starts_with("/web/"))
{
    std::string file = api.substr(5);

    
    if (file.empty())
    {
        
        return GetFolderContent(WEB_ROOT);

    }

    if (FileExists(file, WEB_ROOT))
    {
        return loadHTTPrequest(file, "200 OK");
    }

    return loadHTTPrequest("errors/404.html", "404 Not Found");
}

    std::string apis = std::string(API_ROOT) + "/apis.txt";
    std::string content = readFile(apis);

    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line))
{
    size_t separator = line.find('=');

    if (separator == std::string::npos)
        continue;

    std::string route = line.substr(0, separator);
    std::string command = line.substr(separator + 1);

    if (api == route)
    {
        std::cout << "Route gefunden: " << route << '\n';
        std::cout << "Command: " << command << '\n';

        

        return Execute(command);
    }
}

    return loadHTTPrequest("errors/404.html", "404 Not Found");
}

std::string onload()
{
   if (!FileExists("index.html", WEB_ROOT))
{
    return loadHTTPrequest("errors/502.html", "502 Not Found");
}

return loadHTTPrequest("index.html", "200 OK");
}
