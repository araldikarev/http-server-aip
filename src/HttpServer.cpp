#include "HttpServer.h"
#include "HttpParser.h"
#include "Logger.h"

HttpServer::HttpServer(int port)
    : tcpServer_(port, [this](const std::string& rawRequest) {
        return HandleRequest(rawRequest);
    })
{
}

void HttpServer::AddRoute(
    const std::string& method,
    const std::string& path,
    const HttpHandler& handler
) {
    router_.AddRoute(method, path, handler);
}

void HttpServer::Start() {
    tcpServer_.Start();
}

void HttpServer::Stop() {
    tcpServer_.Stop();
}

std::string HttpServer::HandleRequest(const std::string& rawRequest) {
    try {
        HttpRequest request = HttpParser::Parse(rawRequest);
        HttpResponse response = router_.Route(request);
        return response.to_string();
    }
    catch (const HttpParseException& e) {
        Logger::Warning("Parser error: " + std::string(e.what()));

        HttpResponse response;
        response.status_code = 400;
        response.status_message = "Bad Request";
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        response.body = "<h1>400 Bad Request</h1>";

        return response.to_string();
    }
    catch (const std::exception& e) {
        Logger::Error("Unexpected HTTP handling error: " + std::string(e.what()));

        HttpResponse response;
        response.status_code = 500;
        response.status_message = "Internal Server Error";
        response.headers["Content-Type"] = "text/html; charset=utf-8";
        response.body = "<h1>500 Internal Server Error</h1>";

        return response.to_string();
    }
}