#pragma once

#include <functional>
#include <stdexcept>
#include <winsock2.h>
#include <string>

#include "TcpConnection.h"

class TcpServerException : public std::runtime_error {
public:
    explicit TcpServerException(const std::string &message)
        : std::runtime_error(message) {
    }
};

class TcpServer {
public:
    using ConnectionHandler = std::function<void(TcpConnection &)>;

private:
    int port_;
    ConnectionHandler connectionHandler_;
    bool isRunning_;
    SOCKET listenSocket_;

public:
    TcpServer(int port, ConnectionHandler connectionHandler);

    ~TcpServer();

    void Start();

    void Stop();

    bool IsRunning() const;

    TcpServer(const TcpServer &) = delete;

    TcpServer &operator=(const TcpServer &) = delete;
};
