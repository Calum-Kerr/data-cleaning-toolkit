#ifndef CLEANERS_H
#define CLEANERS_H
#include <vector>
#include <string>
#include <map>

std::vector<std::vector<std::string>> removeDuplicates(const std::vector<std::vector<std::string>>& data);
std::vector<std::vector<std::string>> trimWhitespace(const std::vector<std::vector<std::string>>& data);
std::vector<std::vector<std::string>> standardizeCase(
  const std::vector<std::vector<std::string>>& data, const std::string& caseType);
std::vector<std::vector<std::string>> standardizeNullValuesInData(
  const std::vector<std::vector<std::string>>& data);
std::vector<std::vector<std::string>> fuzzyDeduplicateRows(
  const std::vector<std::vector<std::string>>& data, double threshold);
std::vector<std::vector<std::string>> naturalSort(
  const std::vector<std::vector<std::string>>& data, int colIndex);
std::vector<std::vector<std::string>> removeOutliers(const std::vector<std::vector<std::string>>& data);

struct UniversalCleaningResult {
  std::vector<std::vector<std::string>> cleanedData;
  std::map<int, std::map<std::string, std::string>> columnMappings;
  std::map<int, int> mergedCountPerColumn;
  int duplicateRowsRemoved;
  std::vector<std::string> operationsLog;
};

UniversalCleaningResult universalTextCleaning(
  const std::vector<std::vector<std::string>>& parsed,
  double fuzzyThreshold=0.75,
  bool removeDuplicateRows=true);

#endif

