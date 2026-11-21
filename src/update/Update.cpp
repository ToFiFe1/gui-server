#include "Update.hpp"
#include <files/file_manager.hpp>
#include <api/Apis.hpp>



#include <string>
#include <filesystem>

// Erwartete vorhandene Funktionen:
// std::string readFile(const std::string& path);
// std::string loadHTTPrequest(const std::string& url);
// int saveFile(const std::string& content, const std::string& path);

bool isURL(const std::string& path)
{
    return path.starts_with("http://") ||
           path.starts_with("https://");
}

int GetUpdateFiles(const std::string& path,
                   const std::string& topath)
{
    std::string content;

    // 1. Inhalt laden
    if (isURL(path)) {
        content = loadHTTPrequest(path , "200 OK");
    } else {
        content = readFile(path);
    }

    // 2. Fehler beim Laden prüfen
    if (content.empty()) {
        return -1;
    }

    // 3. Cache-Verzeichnis erstellen
    const std::filesystem::path cachePath =
        std::filesystem::path("cache") /
        std::filesystem::path(topath).filename();

    std::filesystem::create_directories(
        cachePath.parent_path()
    );

    // 4. Inhalt im Cache speichern
    if (saveFile(content, cachePath.string()) != 0) {
        return -2;
    }

    // 5. Zieldatei aktualisieren
    if (saveFile(content, topath) != 0) {
        return -3;
    }

    return 0;
}
bool checkupdateReady(std::string configPath) {}
