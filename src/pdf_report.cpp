#include "pdf_report.h"
#include <sstream>

std::string generateReportJSON(const ReportData& data){
  std::stringstream json;
  json<<"{";
  json<<"\"originalRows\":"<<data.originalData.size()<<",";
  json<<"\"cleanedRows\":"<<data.cleanedData.size()<<",";
  json<<"\"metrics\":{";
  bool first=true;
  for(const auto& pair:data.metrics){
    if(!first) json<<",";
    json<<"\""<<pair.first<<"\":"<<pair.second;
    first=false;
  }
  json<<"},";
  json<<"\"operations\":[";
  first=true;
  for(const auto& op:data.operationsLog){
    if(!first) json<<",";
    json<<"\""<<op<<"\"";
    first=false;
  }
  json<<"]";
  json<<"}";
  return json.str();
}

