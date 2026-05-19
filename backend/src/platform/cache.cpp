#include "cache.h"

static std::map<std::string, CachedFile> fileCache;
static const int CACHE_TTL_SECONDS = 3600;

std::string getCachedFile(const std::string& filepath) {
  auto it = fileCache.find(filepath);
  if (it != fileCache.end()) {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.timestamp).count();
    if (elapsed < CACHE_TTL_SECONDS) {
      return it->second.content;
    }
  }
  return "";
}

void setCachedFile(const std::string& filepath, const std::string& content) {
  CachedFile cached;
  cached.content = content;
  cached.timestamp = std::chrono::system_clock::now();
  fileCache[filepath] = cached;
}

bool isCacheValid(const std::string& filepath) {
  auto it = fileCache.find(filepath);
  if (it == fileCache.end()) return false;
  auto now = std::chrono::system_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.timestamp).count();
  return elapsed < CACHE_TTL_SECONDS;
}
