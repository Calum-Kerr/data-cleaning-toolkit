#ifndef CLUSTERING_H
#define CLUSTERING_H
#include <vector>
#include <string>
#include <map>

struct Cluster {
  int id;
  std::vector<std::string> values;
  int count;
};

struct ClusterResult {
  std::vector<Cluster> clusters;
};

struct MergeMapping {
  int clusterId;
  std::string mergeInto;
};

ClusterResult detectClusters(
  const std::vector<std::vector<std::string>>& data,
  const std::string& column,
  double threshold,
  const std::vector<std::string>& headers);

std::vector<std::vector<std::string>> applyClustering(
  const std::vector<std::vector<std::string>>& data,
  const std::string& column,
  const std::vector<MergeMapping>& merges,
  const std::vector<std::string>& headers);

#endif
