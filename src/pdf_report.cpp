#include "pdf_report.h"
#include <sstream>

std::string generateReportJSON(const ReportData& data){
  std::stringstream json;
  json<<"{\"originalRows\":"<<data.originalData.size();
  json<<",\"cleanedRows\":"<<data.cleanedData.size()<<"}";
  return json.str();
}

