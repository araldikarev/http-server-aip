#pragma once

#include <string>

/// @brief Supported severity levels for log messages.
enum class LogLevel {
    Info,     ///< Informational message.
    Warning,  ///< Recoverable problem or suspicious condition.
    Error     ///< Error message describing a failure.
};

/// @brief Simple console logger for the server.
class Logger {
public:
    /// @brief Writes an informational message.
    /// @param message Message text.
    static void Info(const std::string &message);

    /// @brief Writes a warning message.
    /// @param message Message text.
    static void Warning(const std::string &message);

    /// @brief Writes an error message.
    /// @param message Message text.
    static void Error(const std::string &message);

private:
    /// @brief Writes a message using the selected log level formatting.
    /// @param level Severity level.
    /// @param message Message text.
    static void Log(LogLevel level, const std::string &message);
};
