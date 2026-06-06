#pragma once

#include <functional>
#include <stdexcept>
#include <winsock2.h>
#include <string>

class TcpServerException : public std::runtime_error {
public:
    explicit TcpServerException(const std::string& message)
        : std::runtime_error(message) {}
};

class TcpServer {
public:
    using DataHandler = std::function<std::string(const std::string&)>;
private:
    int port_;
    DataHandler dataHandler_;
    bool isRunning_;
    SOCKET listenSocket_;
public:
    TcpServer(int port, DataHandler dataHandler);
    ~TcpServer();
    void Start();
    void Stop();
    bool IsRunning() const;

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

private:
    std::string Receive(SOCKET clientSocket) const;
    void Send(SOCKET clientSocket, const std::string& data) const;
};
