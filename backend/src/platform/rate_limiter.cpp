#include "rate_limiter.h"
#include <map>
#include <chrono>
#include <mutex>

const int WINDOW_SECONDS=60;
const int MAX_REQUESTS=100;
std::map<std::string, int> requestCounts;
std::map<std::string, std::chrono::steady_clock::time_point> lastReset;
static std::mutex rateLimiterMutex;

void resetIfNeeded(const std::string& ip) {
  auto now=std::chrono::steady_clock::now();
  if (lastReset.find(ip)==lastReset.end()) {
    lastReset[ip]=now;
    requestCounts[ip]=0;
    return;
  }
  auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(now-lastReset[ip]);
  if (elapsed.count()>=WINDOW_SECONDS) {
    lastReset[ip]=now;
    requestCounts[ip]=0;
  }
}

bool checkRateLimit(const std::string& ip) {
  std::lock_guard<std::mutex> lock(rateLimiterMutex);
  resetIfNeeded(ip);
  if (requestCounts[ip]>=MAX_REQUESTS) return false;
  requestCounts[ip]++;
  return true;
}
