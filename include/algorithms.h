#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <map>

struct AuditLogEntry{
    std::string operationName;
    int cellsAffected;
    int rowsBefore;
    int rowsAfter;
    std::string timestamp;
};

class AuditLog{
    public:
    std::vector<AuditLogEntry> entries;
    void addEntry(const std::string& opName,int cellsAffected,int rowsBefore,int rowsAfter){
        auto now=std::chrono::system_clock::now();
        auto time=std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss<<std::ctime(&time);
        std::string timestamp=ss.str();
        timestamp.pop_back(); // remove newline
        entries.push_back({opName, cellsAffected, rowsBefore, rowsAfter, timestamp});
    }
    void clear(){entries.clear();}
};

class DataCleaner{
    public:
    std::vector<std::vector<std::string>> parseCSV(const std::string& data);
    std::vector<std::vector<bool>> detectMissingValues(const std::vector<std::vector<std::string>>& data);
    std::vector<bool> detectDuplicates(const std::vector<std::vector<std::string>>& data);
    std::vector<std::vector<std::string>> cleanData(const std::vector<std::vector<std::string>>& data);
    std::map<std::string,int> profileColumn(const std::vector<std::vector<std::string>>& data, size_t columnIndex);
    std::vector<std::vector<std::string>> standardizeColumnCase(const std::vector<std::vector<std::string>>& data, size_t columnIndex, const std::string& caseType);
    std::vector<std::vector<std::string>> removeEmptyRows(const std::vector<std::vector<std::string>>& data);
    std::vector<std::vector<std::string>> removeDuplicates(const std::vector<std::vector<std::string>>& data);
};

#endif