#pragma once

#include <string>
#include <vector>

struct RequestRecord {
  std::string timestamp;
  std::string method;
  std::string path;
  int statusCode;
};

void initializeDatabase();
void storeRequest(const std::string& timestamp, const std::string& method, const std::string& path, int statusCode);
std::vector<RequestRecord> getRequestHistory(int limit);
