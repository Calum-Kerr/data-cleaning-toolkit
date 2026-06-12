#ifndef TEXT_UTILS_H
#define TEXT_UTILS_H
#include <vector>
#include <string>

std::string toUpperCase(const std::string& text);
std::string toLowerCase(const std::string& text);
std::string normaliseWhitespace(const std::string& text);
std::string normalisePunctuation(const std::string& text);
std::string standardiseNullValues(const std::string& text);
std::string removeStateSuffixes(const std::string& text);
std::string removeDuplicateWords(const std::string& text);

#endif

