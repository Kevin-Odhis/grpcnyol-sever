#pragma once
#include <string>
#include <chrono>

struct LogRecord {
    std::string schoolId;
    std::string userId;
    std::string methodName;
    std::string description;
    long long duration;  // milliseconds
    int statusCode;
    std::string statusMessage;
    std::chrono::system_clock::time_point timestamp;
};