#ifndef DEEP_CLEAN_H
#define DEEP_CLEAN_H

#include <vector>
#include <string>
#include "column_type_detection.h"
#include "../platform/audit.h"

struct DeepCleanResult {
  std::vector<std::vector<std::string>> cleanedData;
  std::vector<ColumnType> columnTypes;
  std::vector<std::string> columnNames;
  AuditLog auditLog;
};

DeepCleanResult deepClean(const std::vector<std::vector<std::string>>& parsed);

#endif
