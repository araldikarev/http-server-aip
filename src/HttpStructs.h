#pragma once

#include <string>
#include <unordered_map>

/// @brief Parsed HTTP request data.
struct HttpRequest {
    std::string method;                                    ///< HTTP method from the request line.
    std::string path;                                      ///< Request target path.
    std::string version;                                   ///< HTTP protocol version.
    std::unordered_map<std::string, std::string> headers;  ///< Request headers.
    std::string body;                                      ///< Request body.
};

/// @brief HTTP response that can be serialized to raw text.
struct HttpResponse {
    int status_code = 200;                                 ///< HTTP status code.
    std::string status_message = "OK";                     ///< HTTP reason phrase.
    std::unordered_map<std::string, std::string> headers;  ///< Response headers.
    std::string body;                                      ///< Response body.

    /// @brief Serializes the response into the HTTP/1.1 wire format.
    /// @return Raw response text containing the status line, headers, and body.
    std::string to_string() const {
        std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " + status_message + "\r\n";

        response += "Content-Length: " + std::to_string(body.length()) + "\r\n";

        for (const auto &[key, value] : headers) {
            if (key == "Content-Length") {
                continue;
            }

            response += key + ": " + value + "\r\n";
        }

        response += "\r\n";
        response += body;
        return response;
    }
};
