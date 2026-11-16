#include "file_manager.hpp"

#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <filesystem>


std::string readFile(const std::string& filename)
{
    std::filesystem::path path(filename);

    if (!path.is_absolute())
    {
        path = std::filesystem::path(WEB_ROOT) / path;
    }

    std::ifstream file(path);

    if (!file)
    {
        std::cerr << "Could not open file: "
                  << path
                  << "\n";

        return "";
    }

    std::cout << "File loaded: "
              << path
              << "\n";

    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}
std::string GetFileType(const std::string& filename)
{
    if (filename.ends_with(".html"))
        return "text/html";

    if (filename.ends_with(".css"))
        return "text/css";

    if (filename.ends_with(".js"))
        return "application/javascript";

    if (filename.ends_with(".json"))
        return "application/json";

    if (filename.ends_with(".png"))
        return "image/png";

    if (filename.ends_with(".jpg") || filename.ends_with(".jpeg"))
        return "image/jpeg";

    if (filename.ends_with(".svg"))
        return "image/svg+xml";

    if (filename.ends_with(".ico"))
        return "image/x-icon";    

    return "application/octet-stream";
}




std::string GetFolderContent(const std::string& folder)
{
    std::string html =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<title>Web Files</title>"
        "</head>"
        "<body>"
        "<h1>Web Files</h1>";

    for (const auto& entry : std::filesystem::directory_iterator(folder))
    {
        std::string filename = entry.path().filename().string();

        html += "<p>";
        html += "<a href=\"/web/" + filename + "\">";
        html += filename;
        html += "</a>";
        html += "</p>";
    }

    html +=
        "</body>"
        "</html>";

    return
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + std::to_string(html.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        html;
}

bool FileExists(const std::string& file, const std::string& folder)
{
    return std::filesystem::exists(
        std::filesystem::path(folder) / file
    );
}