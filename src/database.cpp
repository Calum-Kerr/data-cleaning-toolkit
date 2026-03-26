#include "database.h"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

static const std::string DB_FILE = "/tmp/toolkit.db";

void initializeDatabase() {
  if (!fs::exists(DB_FILE)) {
    std::ofstream file(DB_FILE);
    file << "timestamp,method,path,statusCode\n";
    file.close();
  }
}

void storeRequest(const std::string& timestamp, const std::string& method, const std::string& path, int statusCode) {
  initializeDatabase();
  std::ofstream file(DB_FILE, std::ios::app);
  file << timestamp << "," << method << "," << path << "," << statusCode << "\n";
  file.close();
}

std::vector<RequestRecord> getRequestHistory(int limit) {
  std::vector<RequestRecord> records;
  std::ifstream file(DB_FILE);
  std::string line;
  int count = 0;
  bool firstLine = true;
  while (std::getline(file, line) && count < limit) {
    if (firstLine) {
      firstLine = false;
      continue;
    }
    std::stringstream ss(line);
    std::string timestamp, method, path, statusCodeStr;
    std::getline(ss, timestamp, ',');
    std::getline(ss, method, ',');
    std::getline(ss, path, ',');
    std::getline(ss, statusCodeStr, ',');
    RequestRecord record;
    record.timestamp = timestamp;
    record.method = method;
    record.path = path;
    record.statusCode = std::stoi(statusCodeStr);
    records.push_back(record);
    count++;
  }
  file.close();
  return records;
}
