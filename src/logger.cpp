#include "logger.h"
#include <fstream>

void logRequest(const std::string& method, const std::string& path, int statusCode) {
  std::ofstream log("/tmp/toolkit.log", std::ios::app);
  log << method << " " << path << " " << statusCode << "\n";
}
