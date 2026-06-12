#ifndef WEIGHTED_DEDUP_H
#define WEIGHTED_DEDUP_H

#include <vector>
#include <string>
#include "column_type_detection.h"

struct WeightedDedupResult {
  std::vector<std::vector<std::string>> data;
  int rowsRemoved;
};

WeightedDedupResult weightedDeduplicate(
    const std::vector<std::vector<std::string>>& data,
    const std::vector<ColumnType>& columnTypes,
    double threshold);

#endif
