#include "secure_algorithms.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <set>
#include <map>

bool isValidCSVData(const std::string& data) {
    if (data.find("=+") == 0 || data.find("=-") == 0 || 
        data.find("=@" == 0) || data.find("=") != std::string::npos) {
        size_t pos = data.find('=');
        if (pos != std::string::npos) {
            std::string potentialFormula = data.substr(pos + 1);
            if (potentialFormula.length() > 3) {
                std::string prefix = potentialFormula.substr(0, 3);
                std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper);
                if (prefix == "SUM" || prefix == "AVG" || prefix == "MAX" || prefix == "MIN") {
                    return false; 
                }
            }
        }
    }
    
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.length() > 10000) { // 10k character limit per line
            return false;
        }
    }
    
    return true;
}

bool isNumericSecure(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    if (start >= str.length()) return false;
    
    bool hasDecimal = false;
    for (size_t i = start; i < str.length(); i++) {
        if (str[i] == '.') {
            if (hasDecimal) return false; 
            hasDecimal = true;
        } else if (str[i] < '0' || str[i] > '9') {
            return false;
        }
    }
    return true;
}

bool isDateFormatSecure(const std::string& str) {
    if (str.length() < 6 || str.length() > 10) return false;
    
    int digits = 0, separators = 0;
    char sep = '\0';
    for (char c : str) {
        if (c >= '0' && c <= '9') {
            digits++;
        } else if (c == '/' || c == '-' || c == '.') {
            separators++;
            if (sep == '\0') sep = c;
        } else {
            return false; 
        }
    }
    
    return digits >= 4 && digits <= 8 && separators == 2;
}

std::string standardiseDateToISO_secure(const std::string& str) {
    if (str.length() < 6) return str;
    char sep = '/';
    if (str.find('-') != std::string::npos) sep = '-';
    else if (str.find('.') != std::string::npos) sep = '.';
    std::vector<std::string> parts;
    std::stringstream ss(str);
    std::string part;
    while (std::getline(ss, part, sep)) {
        parts.push_back(part);
    }
    if (parts.size() != 3) return str;
    std::string year, month, day;
    if (parts[0].length() == 4) {
        year = parts[0];
        month = parts[1];
        day = parts[2];
    } else if (parts[2].length() == 4) {
        // DD/MM/YYYY or MM/DD/YYYY format
        if (std::stoi(parts[0]) > 12) {
            // Day first: DD/MM/YYYY
            day = parts[0];
            month = parts[1];
            year = parts[2];
        } else if (std::stoi(parts[1]) > 12) {
            // Month first: MM/DD/YYYY
            month = parts[0];
            day = parts[1];
            year = parts[2];
        } else {
            // Ambiguous, default to DD/MM/YYYY
            day = parts[0];
            month = parts[1];
            year = parts[2];
        }
    } else {
        return str; 
    }
    try {
        int day_int = std::stoi(day);
        int month_int = std::stoi(month);
        int year_int = std::stoi(year);
        if (day_int < 1 || day_int > 31 || month_int < 1 || month_int > 12) {
            return str; 
        }
    } catch (...) {
        return str; 
    }
    
    if (month.length() == 1) month = "0" + month;
    if (day.length() == 1) day = "0" + day;
    
    return year + "-" + month + "-" + day;
}

std::string standardiseNumberSecure(const std::string& str) {
    if (str.empty()) return str;
    
    std::string result;
    bool hasDecimal = false;
    int commaCount = 0, periodCount = 0;
    
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == ',') commaCount++;
        else if (str[i] == '.') periodCount++;
    }
    char decimalSep = '.';
    if (commaCount > periodCount) decimalSep = ',';
    for (size_t i = 0; i < str.length(); i++) {
        char c = str[i];
        if (c >= '0' && c <= '9') {
            result += c;
        } else if ((c == '.' || c == ',') && !hasDecimal) {
            if (c == decimalSep) {
                result += '.';
                hasDecimal = true;
            }
        } else if (c == '-' && result.empty()) {
            // Allow negative sign at start
            result += c;
        }
    }
    
    return result.empty() ? str : result;
}

std::vector<std::vector<std::string>> parseCSVSecure(const std::string& data) {
    std::vector<std::vector<std::string>> result;
    
    if (!isValidCSVData(data)) {
        throw std::runtime_error("Invalid CSV data detected");
    }
    
    std::istringstream ss(data);
    std::string line;
    size_t lineCount = 0;
    
    while (std::getline(ss, line)) {
        lineCount++;
        if (lineCount > 10000) { 
            break;
        }
        
        std::vector<std::string> row;
        std::istringstream lineStream(line);
        std::string cell;
        size_t cellCount = 0;
        
        size_t pos = 0, prev = 0;
        while ((pos = line.find(',', pos)) != std::string::npos) {
            cellCount++;
            if (cellCount > 1000) { // Limit to 1000 columns to prevent DoS
                break;
            }
            
            std::string cell = line.substr(prev, pos - prev);
            row.push_back(cell);
            pos++;
            prev = pos;
        }
        
        if (prev < line.length()) {
            cellCount++;
            if (cellCount <= 1000) { // Only add if we're still under the limit
                std::string cell = line.substr(prev);
                row.push_back(cell);
            }
        }
        
        if (!row.empty()) {
            result.push_back(row);
        }
    }
    
    return result;
}