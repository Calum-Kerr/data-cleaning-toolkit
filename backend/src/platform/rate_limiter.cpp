#include "rate_limiter.h"
#include <map>
#include <chrono>
#include <mutex>

const int WINDOW_SECONDS=60;
const int MAX_REQUESTS=100;
std::map<std::string, int> requestCounts;
std::map<std::string, std::chrono::steady_clock::time_point> lastReset;
static std::mutex rateLimiterMutex;

// Per-IP concurrency tracking
static std::map<std::string, int> activeConnections;
static std::mutex concurrencyMutex;

std::string resolveClientIp(const std::string& xffHeader, const std::string& remoteIp) {
  if (xffHeader.empty()) return remoteIp;
  // Only the last entry was appended by our own trusted proxy.
  auto comma=xffHeader.find_last_of(',');
  std::string candidate=comma==std::string::npos?xffHeader:xffHeader.substr(comma+1);
  auto begin=candidate.find_first_not_of(" \t");
  if (begin==std::string::npos) return remoteIp;
  auto end=candidate.find_last_not_of(" \t");
  return candidate.substr(begin, end-begin+1);
}

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

void pruneStaleEntries() {
  auto now=std::chrono::steady_clock::now();
  for (auto it=lastReset.begin(); it!=lastReset.end(); ) {
    auto elapsed=std::chrono::duration_cast<std::chrono::seconds>(now-it->second);
    if (elapsed.count()>WINDOW_SECONDS*2) {
      requestCounts.erase(it->first);
      it=lastReset.erase(it);
    } else {
      ++it;
    }
  }
}

bool checkRateLimit(const std::string& ip) {
  std::lock_guard<std::mutex> lock(rateLimiterMutex);
  static int callCount=0;
  if (++callCount%10==0) {
    pruneStaleEntries();
  }
  resetIfNeeded(ip);
  if (requestCounts[ip]>=MAX_REQUESTS) return false;
  requestCounts[ip]++;
  return true;
}

bool tryAcquireConnection(const std::string& ip) {
  std::lock_guard<std::mutex> lock(concurrencyMutex);
  if (activeConnections[ip] >= MAX_CONCURRENT_PER_IP) return false;
  activeConnections[ip]++;
  return true;
}

void releaseConnection(const std::string& ip) {
  std::lock_guard<std::mutex> lock(concurrencyMutex);
  auto it = activeConnections.find(ip);
  if (it != activeConnections.end()) {
    if (it->second > 0) it->second--;
  }
}
