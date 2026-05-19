#pragma once

#include <string>

void recordPageMetric(const std::string& path, int statusCode, long responseTime);
void writeSeoReport();
