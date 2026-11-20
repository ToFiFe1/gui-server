#pragma once
#include <string>

int GetUpdateFiles(std::string webURl, std::string path);

int UpdateFilesCached(std::string method,std::string configPath); // "-hard" update all, "-medium" update all changed files, "-light" : all the files in this config / modules / Config / preset1_bit

