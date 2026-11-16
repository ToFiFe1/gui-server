#pragma once
#include <iostream>

std::string handleRequest(const std::string& api);

std::string onload();

int sentX();

std::string GetFolderContent(const std::string& folder);

std::string loadHTTPrequest(
    const std::string& filename,
    const std::string& status
);
std::string apiResponse(const std::string& body);