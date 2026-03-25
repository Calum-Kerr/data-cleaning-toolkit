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

std::string getContentType(const std::string& filepath) {
  if (endsWith(filepath, ".html")) return "text/html; charset=utf-8";
  if (endsWith(filepath, ".css")) return "text/css; charset=utf-8";
  if (endsWith(filepath, ".js")) return "application/javascript; charset=utf-8";
  if (endsWith(filepath, ".json")) return "application/json; charset=utf-8";
  if (endsWith(filepath, ".wasm")) return "application/wasm";
  if (endsWith(filepath, ".png")) return "image/png";
  if (endsWith(filepath, ".ico")) return "image/x-icon";
  if (endsWith(filepath, ".svg")) return "image/svg+xml";
  if (endsWith(filepath, ".webmanifest")) return "application/manifest+json";
  if (endsWith(filepath, ".xml")) return "application/xml; charset=utf-8";
  if (endsWith(filepath, ".txt")) return "text/plain; charset=utf-8";
  return "application/octet-stream";
}





