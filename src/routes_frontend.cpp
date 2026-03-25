#include "crow_all.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

bool endsWith(const std::string& str, const std::string& suffix) {
  if (str.length() < suffix.length()) return false;
  return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}


