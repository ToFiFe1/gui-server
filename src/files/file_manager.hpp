#pragma once
#include <string>
std::string readFile(const std::string& filename);
std::string GetFileType(const std::string& filename);
bool FileExists(const std::string& file, const std::string& folder);
int saveFile(const std::string& content, const std::string& path);