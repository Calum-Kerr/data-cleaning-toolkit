#include "analytics.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

static std::map<std::string, int> endpointCalls;

void recordEndpointCall(const std::string& endpoint) {
  endpointCalls[endpoint]++;
}

void writeAnalyticsSummary() {
  std::ofstream file("/tmp/toolkit_analytics.log", std::ios::app);
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
  file << "ANALYTICS SUMMARY " << ss.str() << "\n";
  for (const auto& pair : endpointCalls) {
    file << pair.first << ": " << pair.second << " calls\n";
  }
  file << "\n";
}
