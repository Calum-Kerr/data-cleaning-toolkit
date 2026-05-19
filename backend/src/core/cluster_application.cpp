#include "cluster_detection.h"
#include <map>
#include <set>

extern ClusterResult detectClusters(const std::vector<std::vector<std::string>>& data,
  const std::string& column, double threshold, const std::vector<std::string>& headers);

std::vector<std::vector<std::string>> applyClustering(
  const std::vector<std::vector<std::string>>& data,
  const std::string& column,
  const std::vector<MergeMapping>& merges,
  const std::vector<std::string>& headers) {
  int colIndex = -1;
  for(size_t i = 0; i < headers.size(); i++) {
    if(headers[i] == column) { colIndex = i; break; }
  }
  std::vector<std::vector<std::string>> result = data;
  if(colIndex < 0) return result;
  std::map<std::string, std::string> valueMapping;
  for(const auto& m : merges) {
    for(const auto& val : m.values) {
      valueMapping[val] = m.mergeInto;
    }
  }
  for(auto& row : result) {
    if(colIndex < (int)row.size()) {
      auto it = valueMapping.find(row[colIndex]);
      if(it != valueMapping.end()) row[colIndex] = it->second;
    }
  }
  return result;
}
