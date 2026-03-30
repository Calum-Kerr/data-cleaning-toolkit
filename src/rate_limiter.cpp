#include "rate_limiter.h"
#include <map>
#include <chrono>

const int WINDOW_SECONDS=60;
const int MAX_REQUESTS=100;
std::map<std::string, int> requestCounts;
std::map<std::string, std::chrono::steady_clock::time_point> lastReset;

bool checkRateLimit(const std::string& ip) {
  requestCounts[ip]++;
  if (requestCounts[ip] > 100) return false;
  return true;
}
