#include "load_test.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

LoadTestResult simulateLoad(int requestCount) {
  LoadTestResult result;
  result.totalRequests = requestCount;
  result.successfulRequests = requestCount;
  result.averageResponseTime = 50;
  result.minResponseTime = 10;
  result.maxResponseTime = 150;
  return result;
}

void writeLoadTestReport(const LoadTestResult& result) {
  std::ofstream file("/tmp/toolkit_load_test.log", std::ios::app);
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
  file << "LOAD TEST REPORT " << ss.str() << "\n";
  file << "Total Requests: " << result.totalRequests << "\n";
  file << "Successful Requests: " << result.successfulRequests << "\n";
  file << "Average Response Time: " << result.averageResponseTime << "ms\n";
  file << "Min Response Time: " << result.minResponseTime << "ms\n";
  file << "Max Response Time: " << result.maxResponseTime << "ms\n";
  file << "\n";
}
