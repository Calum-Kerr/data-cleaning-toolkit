#include "clustering.h"
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
  std::map<int, std::set<std::string>> mergeMap;
  for(const auto& m : merges) mergeMap[m.clusterId].insert(m.mergeInto);
  auto clusters = detectClusters(data, column, 0.75, headers);
  std::map<std::string, std::string> valueMapping;
  for(const auto& cluster : clusters.clusters) {
    if(mergeMap.find(cluster.id) != mergeMap.end()) {
      const std::string& mergeInto = *mergeMap[cluster.id].begin();
      for(const auto& val : cluster.values) valueMapping[val] = mergeInto;
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
