#ifndef COLUMN_TYPE_DETECTION_H
#define COLUMN_TYPE_DETECTION_H

#include <vector>
#include <string>

enum class ColumnType {
  EMAIL,
  PHONE,
  URL,
  DATE,
  NUMERIC,
  BOOLEAN,
  ID,
  NAME,
  FREE_TEXT,
  GENERIC_TEXT
};

struct ColumnTypeResult {
  std::vector<ColumnType> types;
  std::vector<std::string> names;
};

ColumnTypeResult detectColumnTypes(const std::vector<std::vector<std::string>>& data);
const char* columnTypeToString(ColumnType t);
double typeWeight(ColumnType t);

#endif
