#ifndef LOGGER_H
#define LOGGER_H
#include <string>

void logRequest(const std::string& method, const std::string& path, int statusCode);

#endif
