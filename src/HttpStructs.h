#pragma once
#include <string>
#include <unordered_map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    int status_code = 200;
    std::string status_message = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string to_string() const {
        std::string response = "HTTP/1.1 " + std::to_string(status_code) + " " + status_message + "\r\n";

        const std::string response_body = body;
        response += "Content-Length: " + std::to_string(response_body.length()) + "\r\n";

        for (const auto &[key, value]: headers) {
            response += key + ": " + value + "\r\n";
        }
        response += "\r\n";
        response += response_body;
        return response;
    }
};
