#pragma once

#include <string>

#include "Router.h"
#include "TcpServer.h"

/// @brief High-level HTTP server that combines routing and low-level TCP transport.
class HttpServer {
private:
    Router router_;        ///< Route table used to resolve parsed HTTP requests.
    TcpServer tcpServer_;  ///< Low-level TCP server that accepts client connections.

public:
    /// @brief Creates an HTTP server bound to the specified TCP port.
    /// @param port Listening port.
    explicit HttpServer(int port);

    /// @brief Registers a route handler.
    /// @param method HTTP method such as "GET" or "POST".
    /// @param path URL path to match.
    /// @param handler Callback used to build the response.
    void AddRoute(const std::string &method, const std::string &path, const HttpHandler &handler);

    /// @brief Starts accepting and processing client connections.
    void Start();

    /// @brief Requests the TCP server to stop processing new connections.
    void Stop();

private:
    /// @brief Processes a single accepted TCP connection.
    /// @param connection Active client connection.
    void HandleConnection(TcpConnection &connection);

    /// @brief Receives a complete HTTP request from a client connection.
    /// @param connection Active client connection.
    /// @return Raw HTTP request text including headers and body.
    std::string ReceiveHttpRequest(TcpConnection &connection) const;

    /// @brief Builds a serialized HTTP response for a raw request string.
    /// @param rawRequest Raw HTTP request text.
    /// @return Raw HTTP response text.
    std::string BuildResponse(const std::string &rawRequest);
};
