#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <algorithm>
#include <stdexcept>
class SecureBuffer {
private:
    std::unique_ptr<char[]> data;
    size_t size;
    
public:
    explicit SecureBuffer(size_t s) : size(s) {
        if (size > 100 * 1024 * 1024) { // 100MB limit to prevent resource exhaustion
            throw std::runtime_error("Input too large");
        }
        data = std::make_unique<char[]>(size);
        std::fill(data.get(), data.get() + size, 0); 
    }
    ~SecureBuffer() {
        if (data) {
            secure_wipe(data.get(), size);
        }
    }
    
    char* get() { return data.get(); }
    size_t get_size() const { return size; }
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;
    SecureBuffer(SecureBuffer&&) = default;
    SecureBuffer& operator=(SecureBuffer&&) = default;
    
private:
    void secure_wipe(char* ptr, size_t len) {
        if (ptr) {
            volatile char* p = ptr;
            for (size_t i = 0; i < len; ++i) {
                p[i] = 0;
            }
        }
    }
};

class SecureString {
private:
    std::string data;
    
public:
    explicit SecureString(const std::string& str) {
        if (str.length() > 50 * 1024 * 1024) { // 50MB limit
            throw std::runtime_error("String too large");
        }
        data = str;
    }
    
    const char* c_str() const { return data.c_str(); }
    size_t length() const { return data.length(); }
    bool empty() const { return data.empty(); }
    
    char at(size_t pos) const {
        if (pos >= data.length()) {
            throw std::out_of_range("SecureString access out of bounds");
        }
        return data[pos];
    }
    
    std::string substr(size_t pos, size_t len = std::string::npos) const {
        return data.substr(pos, len);
    }
    
    bool equals(const std::string& other) const {
        if (data.length() != other.length()) {
            return false;
        }
        
        volatile int result = 0;
        for (size_t i = 0; i < data.length(); ++i) {
            result |= data[i] ^ other[i];
        }
        return result == 0;
    }
};

bool isValidCSVData(const std::string& data);
bool isNumericSecure(const std::string& str);
bool isDateFormatSecure(const std::string& str);
std::string standardiseDateToISO_secure(const std::string& str);
std::string standardiseNumberSecure(const std::string& str);
std::vector<std::vector<std::string>> parseCSVSecure(const std::string& data);