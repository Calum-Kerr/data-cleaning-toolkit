#ifndef AUDIT_H
#define AUDIT_H
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include "time_safe.h"

struct AuditLogEntry {
  std::string operationName;
  int cellsAffected;
  int rowsBefore;
  int rowsAfter;
  std::string timestamp;
};

class AuditLog {
public:
  std::vector<AuditLogEntry> entries;
  void addEntry(const std::string& opName, int cellsAffected, int rowsBefore, int rowsAfter){
    auto now=std::chrono::system_clock::now();
    auto time=std::chrono::system_clock::to_time_t(now);
    std::string timestamp=ctime_safe(time);
    entries.push_back({opName, cellsAffected, rowsBefore, rowsAfter, timestamp});
  }
  void clear(){entries.clear();}
};

#endif

