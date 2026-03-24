#ifndef DETECTORS_H
#define DETECTORS_H
#include <vector>
#include <string>
#include <map>

int levenshteinDistance(const std::string& s1, const std::string& s2);
double calculateSimilarity(const std::string& s1, const std::string& s2);
std::vector<std::vector<bool>> detectMissingValues(const std::vector<std::vector<std::string>>& data);
std::vector<bool> detectDuplicates(const std::vector<std::vector<std::string>>& data);
std::vector<int> detectOutliers(const std::vector<std::vector<std::string>>& data);
std::map<std::string, std::vector<std::string>> detectInconsistentValues(
  const std::vector<std::vector<std::string>>& data);

#endif

