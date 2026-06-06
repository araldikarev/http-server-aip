#include "HttpServer.h"

#include <algorithm>
#include <cctype>
#include <string>

#include "HttpParser.h"
#include "Logger.h"

namespace {
    std::string ToLower(std::string value) {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            }
        );
        return value;
    }

    std::string TrimCopy(std::string value) {
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
            value.erase(value.begin());
        }
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
            value.pop_back();
        }
        return value;
    }

    size_t ExtractContentLength(const std::string &headersPart) {
        const std::string lowered = ToLower(headersPart);
        const std::string headerName = "content-length:";

        const size_t pos = lowered.find(headerName);
        if (pos == std::string::npos) {
            return 0;
        }

        const size_t valueStart = pos + headerName.length();
        const size_t valueEnd = headersPart.find("\r\n", valueStart);

        std::string value = valueEnd == std::string::npos
                                ? headersPart.substr(valueStart)
                                : headersPart.substr(valueStart, valueEnd - valueStart);

        value = TrimCopy(value);

        if (value.empty()) {
            throw HttpParseException("Invalid Content-Length header");
        }

        try {
            return static_cast<size_t>(std::stoul(value));
        } catch (const std::exception &) {
            throw HttpParseException("Invalid Content-Length header");
        }
    }
}

HttpServer::HttpServer(int port)
    : tcpServer_(port, [this](TcpConnection &connection) {
        HandleConnection(connection);
    }) {
}

void HttpServer::AddRoute(
    const std::string &method,
    const std::string &path,
    const HttpHandler &handler
) {
    router_.AddRoute(method, path, handler);
}

void HttpServer::Start() {
    tcpServer_.Start();
}

void HttpServer::Stop() {
    tcpServer_.Stop();
}

void HttpServer::HandleConnection(TcpConnection &connection) {
    try {
        const std::string rawRequest = ReceiveHttpRequest(connection);
        const std::string rawResponse = BuildResponse(rawRequest);

        connection.SendAll(rawResponse);

        Logger::Info("Response sent successfully");
    } catch (const HttpParseException &e) {
        Logger::Warning("HTTP request error: " + std::string(e.what()));

        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        response.body = "<h1>400 Bad Request</h1>";

        connection.SendAll(response.to_string());
    }
}

std::string HttpServer::ReceiveHttpRequest(TcpConnection &connection) const {
    constexpr size_t MaxHeaderSize = 16 * 1024;
    constexpr size_t MaxBodySize = 64 * 1024;
    constexpr size_t MaxRequestSize = MaxHeaderSize + MaxBodySize;

    std::string data;
    size_t headerEnd = std::string::npos;

    while ((headerEnd = data.find("\r\n\r\n")) == std::string::npos) {
        data += connection.ReceiveChunk();

        if (data.size() > MaxHeaderSize) {
            throw HttpParseException("Request headers are too large");
        }
    }

    const size_t bodyStart = headerEnd + 4;
    const std::string headersPart = data.substr(0, headerEnd);

    const size_t contentLength = ExtractContentLength(headersPart);

    if (contentLength > MaxBodySize) {
        throw HttpParseException("Request body is too large");
    }

    while (data.size() - bodyStart < contentLength) {
        data += connection.ReceiveChunk();

        if (data.size() > MaxRequestSize) {
            throw HttpParseException("Request is too large");
        }
    }

    return data;
}

std::string HttpServer::BuildResponse(const std::string &rawRequest) {
    try {
        HttpRequest request = HttpParser::Parse(rawRequest);
        HttpResponse response = router_.Route(request);
        return response.to_string();
    } catch (const HttpParseException &e) {
        Logger::Warning("Parser error: " + std::string(e.what()));

        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        response.body = "<h1>400 Bad Request</h1>";

        return response.to_string();
    } catch (const std::exception &e) {
        Logger::Error("Unexpected HTTP handling error: " + std::string(e.what()));

        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        response.body = "<h1>500 Internal Server Error</h1>";

        return response.to_string();
    }
}
