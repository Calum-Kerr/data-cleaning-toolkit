#include "logger.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "time_safe.h"

std::string getTimestamp() {
  auto now=std::chrono::system_clock::now();
  auto t=std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  std::tm tm=localtime_safe(t);
  ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  return ss.str();
}

// TODO: Move log directory from /tmp to /var/lib/toolkit/ with 0700 permissions
// to prevent log file enumeration by other local processes.  Set the path via
// an environment variable (TMPDIR or TOOLKIT_LOG_DIR) for deploy-time override.
void logRequest(const std::string& method, const std::string& path, int statusCode) {
  std::ofstream log("/tmp/toolkit.log", std::ios::app);
  log << getTimestamp() << " " << method << " " << path << " " << statusCode << "\n";
}
