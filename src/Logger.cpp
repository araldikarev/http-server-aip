#include "Logger.h"
#include <iostream>

void Logger::Info(const std::string& message) {
    Log(LogLevel::Info, message);
}

void Logger::Warning(const std::string& message) {
    Log(LogLevel::Warning, message);
}

void Logger::Error(const std::string& message) {
    Log(LogLevel::Error, message);
}

void Logger::Log(LogLevel level, const std::string& message) {
    switch (level) {
        case LogLevel::Info:
            std::cout << "[INFO] " << message << std::endl;
            break;

        case LogLevel::Warning:
            std::cout << "[WARNING] " << message << std::endl;
            break;

        case LogLevel::Error:
            std::cerr << "[ERROR] " << message << std::endl;
            break;
    }
}