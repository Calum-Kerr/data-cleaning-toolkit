#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H
#include <string>

bool checkRateLimit(const std::string& ip);

// Per-IP connection concurrency limiting (defence against memory-exhaustion DoS).
// Maximum simultaneous requests from a single IP.  Returns false if at limit.
static const int MAX_CONCURRENT_PER_IP = 5;
bool tryAcquireConnection(const std::string& ip);
void releaseConnection(const std::string& ip);

// RAII guard that releases the connection slot when it goes out of scope.
class ConnectionGuard {
public:
  ConnectionGuard(const std::string& ip) : ip_(ip), held_(true) {}
  ~ConnectionGuard() { if(held_) releaseConnection(ip_); }
  void dismiss() { held_ = false; }  // call if the slot was never actually held
private:
  std::string ip_;
  bool held_;
};

#endif
