#include "rate_limiter.h"
#include <map>
#include <chrono>

std::map<std::string, int> requestCounts;

bool checkRateLimit(const std::string& ip) {
  requestCounts[ip]++;
  if (requestCounts[ip] > 100) return false;
  return true;
}
