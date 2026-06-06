#pragma once
#include "Router.h"
#include "TcpServer.h"


class HttpServer {
private:
    Router router_;
    TcpServer tcpServer_;
public:
    explicit HttpServer(int port);

    void AddRoute(
        const std::string& method,
        const std::string& path,
        const HttpHandler& handler
    );

    void Start();
    void Stop();

private:
    std::string HandleRequest(const std::string& rawRequest);
};

