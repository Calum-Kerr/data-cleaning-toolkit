#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

bool endsWith(const std::string& str, const std::string& suffix) {
  if (str.length() < suffix.length()) return false;
  return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string readFile(const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}



