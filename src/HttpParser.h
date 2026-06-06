#pragma once

#include <stdexcept>

#include "HttpStructs.h"

/// @brief Exception thrown when an HTTP request cannot be parsed.
class HttpParseException : public std::runtime_error {
public:
    /// @brief Creates a parsing exception with a readable message.
    /// @param message Error description.
    explicit HttpParseException(const std::string &message) : std::runtime_error(message) {}
};

/// @brief Parses raw HTTP request text into a structured representation.
class HttpParser {
public:
    /// @brief Parses a raw HTTP request string into HttpRequest.
    /// @param rawRequest Full request text including headers and optional body.
    /// @return Parsed request structure.
    /// @throws HttpParseException If the request is malformed or unsupported.
    static HttpRequest Parse(const std::string &rawRequest);
};
