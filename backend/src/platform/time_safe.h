#ifndef TIME_SAFE_H
#define TIME_SAFE_H

#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

// Thread-safe wrappers around localtime and gmtime.
// Returns tm by value to avoid shared static buffers.

inline std::tm localtime_safe(std::time_t t) {
  std::tm result;
#ifdef _WIN32
  localtime_s(&result, &t);
#else
  localtime_r(&t, &result);
#endif
  return result;
}

inline std::tm gmtime_safe(std::time_t t) {
  std::tm result;
#ifdef _WIN32
  gmtime_s(&result, &t);
#else
  gmtime_r(&t, &result);
#endif
  return result;
}

// Format a tm struct as a C-style ctime string (no trailing newline).
inline std::string ctime_safe(std::time_t t) {
  std::tm tm = localtime_safe(t);
  std::stringstream ss;
  ss << std::put_time(&tm, "%a %b %d %H:%M:%S %Y");
  return ss.str();
}

#endif
