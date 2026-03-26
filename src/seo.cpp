#include "seo.h"
#include <fstream>
#include <map>
#include <chrono>
#include <iomanip>
#include <sstream>

struct PageMetrics {
  int statusCode;
  long responseTime;
  std::chrono::system_clock::time_point timestamp;
};

static std::map<std::string, PageMetrics> pageMetrics;

void recordPageMetric(const std::string& path, int statusCode, long responseTime) {
  PageMetrics metric;
  metric.statusCode = statusCode;
  metric.responseTime = responseTime;
  metric.timestamp = std::chrono::system_clock::now();
  pageMetrics[path] = metric;
}

void writeSeoReport() {
  std::ofstream file("/tmp/toolkit_seo.log", std::ios::app);
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
  file << "SEO REPORT " << ss.str() << "\n";
  for (const auto& pair : pageMetrics) {
    file << pair.first << " Status: " << pair.second.statusCode << " ResponseTime: " << pair.second.responseTime << "ms\n";
  }
  file << "\n";
}
