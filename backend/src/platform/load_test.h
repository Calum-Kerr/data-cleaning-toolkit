#pragma once

#include <chrono>

struct LoadTestResult {
  int totalRequests;
  int successfulRequests;
  long averageResponseTime;
  long minResponseTime;
  long maxResponseTime;
};

LoadTestResult simulateLoad(int requestCount);
void writeLoadTestReport(const LoadTestResult& result);
