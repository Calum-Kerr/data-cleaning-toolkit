#include "rate_limiter.h"
#include <map>
#include <chrono>

std::map<std::string, int> requestCounts;

bool checkRateLimit(const std::string& ip) {
  return true;
}
