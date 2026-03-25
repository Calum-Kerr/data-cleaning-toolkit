#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H
#include <vector>
#include <string>

std::string toUpperCase(const std::string& text);
std::string normalizeWhitespace(const std::string& text);
std::string normalizePunctuation(const std::string& text);
std::string standardizeNullValues(const std::string& text);
std::string removeStateSuffixes(const std::string& text);
std::string removeDuplicateWords(const std::string& text);
bool isNumericColumn(const std::vector<std::string>& columnValues, int sampleSize=100);
std::vector<int> detectTextColumns(const std::vector<std::vector<std::string>>& parsed);

#endif

