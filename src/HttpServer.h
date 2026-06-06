#pragma once
#include "Router.h"
#include "TcpServer.h"

#include <string>

class HttpServer {
private:
    Router router_;
    TcpServer tcpServer_;

public:
    explicit HttpServer(int port);

    void AddRoute(
        const std::string &method,
        const std::string &path,
        const HttpHandler &handler
    );

    void Start();

    void Stop();

private:
    void HandleConnection(TcpConnection &connection);

    std::string ReceiveHttpRequest(TcpConnection &connection) const;

    std::string BuildResponse(const std::string &rawRequest);
};
