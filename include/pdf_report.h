#ifndef PDF_REPORT_H
#define PDF_REPORT_H
#include <vector>
#include <string>
#include <map>

struct ReportData {
  std::vector<std::vector<std::string>> originalData;
  std::vector<std::vector<std::string>> cleanedData;
  std::map<std::string, int> metrics;
  std::vector<std::string> operationsLog;
};

std::string generateReportJSON(const ReportData& data);

#endif

