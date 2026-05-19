#pragma once

#include <string>
#include <map>
#include <chrono>

struct CachedFile {
  std::string content;
  std::chrono::system_clock::time_point timestamp;
};

std::string getCachedFile(const std::string& filepath);
void setCachedFile(const std::string& filepath, const std::string& content);
bool isCacheValid(const std::string& filepath);
