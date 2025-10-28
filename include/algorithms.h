#ifndef ALGORITHMS_H
#define ALGORITHMS_H
#include <vector>
#include <string>

class DataCleaner{
    public:
    std::vector<std::vector<std::string>> parseCSV(const std::string& data);
    std::vector<std::vector<bool>> detectMissingValues(const std::vector<std::vector<std::string>>& data);
    std::vector<bool> detectDuplicates(const std::vector<std::vector<std::string>>& data);
    std::vector<std::vector<std::string>> cleanData(const std::vector<std::vector<std::string>>& data);
};

#endif