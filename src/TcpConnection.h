#pragma once
#include <stdexcept>
#include <winsock2.h>
#include <string>

class TcpConnectionException : public std::runtime_error {
public:
    explicit TcpConnectionException(const std::string &message)
        : std::runtime_error(message) {
    }
};

class TcpConnection {
private:
    SOCKET socket_;

public:
    explicit TcpConnection(SOCKET socket);

    ~TcpConnection();

    TcpConnection(const TcpConnection &) = delete;

    TcpConnection &operator=(const TcpConnection &) = delete;

    TcpConnection(TcpConnection &&other) noexcept;

    TcpConnection &operator=(TcpConnection &&other) noexcept;

    std::string ReceiveChunk(size_t bufferSize = 2048) const;

    void SendAll(const std::string &message) const;

    bool IsValid() const;
};
