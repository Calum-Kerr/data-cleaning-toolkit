#include "clustering.h"
#include "detectors.h"
#include <map>

ClusterResult detectClusters(const std::vector<std::vector<std::string>>& data,
  const std::string& column, double threshold, const std::vector<std::string>& headers) {
  ClusterResult result;
  int colIndex = -1;
  for(size_t i = 0; i < headers.size(); i++) {
    if(headers[i] == column) { colIndex = i; break; }
  }
  if(colIndex < 0) return result;
  std::map<std::string, int> valueFreq;
  for(const auto& row : data) {
    if(colIndex < (int)row.size()) valueFreq[row[colIndex]]++;
  }
  std::vector<std::string> uniqueValues;
  for(const auto& pair : valueFreq) uniqueValues.push_back(pair.first);
  std::vector<bool> clustered(uniqueValues.size(), false);
  int clusterId = 0;
  for(size_t i = 0; i < uniqueValues.size(); i++) {
    if(clustered[i]) continue;
    Cluster cluster;
    cluster.id = clusterId++;
    cluster.values.push_back(uniqueValues[i]);
    clustered[i] = true;
    for(size_t j = i + 1; j < uniqueValues.size(); j++) {
      if(clustered[j]) continue;
      if(calculateSimilarity(uniqueValues[i], uniqueValues[j]) >= threshold) {
        cluster.values.push_back(uniqueValues[j]);
        clustered[j] = true;
      }
    }
    for(const auto& val : cluster.values) cluster.count += valueFreq[val];
    result.clusters.push_back(cluster);
  }
  return result;
}
