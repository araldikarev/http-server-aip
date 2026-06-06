#include "TcpConnection.h"

#include <utility>

TcpConnection::TcpConnection(SOCKET socket)
    : socket_(socket) {
}

TcpConnection::~TcpConnection() {
    if (socket_ != INVALID_SOCKET) {
        closesocket(socket_);
    }
}

TcpConnection::TcpConnection(TcpConnection &&other) noexcept
    : socket_(other.socket_) {
    other.socket_ = INVALID_SOCKET;
}

TcpConnection &TcpConnection::operator=(TcpConnection &&other) noexcept {
    if (this != &other) {
        if (socket_ != INVALID_SOCKET) {
            closesocket(socket_);
        }

        socket_ = other.socket_;
        other.socket_ = INVALID_SOCKET;
    }

    return *this;
}

std::string TcpConnection::ReceiveChunk(size_t bufferSize) const {
    std::string buffer(bufferSize, '\0');

    const int bytesReceived = recv(socket_, buffer.data(), static_cast<int>(buffer.size()), 0);

    if (bytesReceived > 0) {
        buffer.resize(bytesReceived);
        return buffer;
    }

    if (bytesReceived == 0) {
        throw TcpConnectionException("Client closed connection cleanly.");
    }

    throw TcpConnectionException(
        "Receive failed with error: " + std::to_string(WSAGetLastError())
    );
}

void TcpConnection::SendAll(const std::string &data) const {
    int totalSent = 0;
    const int dataSize = static_cast<int>(data.size());
    while (totalSent < dataSize) {
        const int sent = send(
            socket_,
            data.c_str() + totalSent,
            dataSize - totalSent,
            0
        );
        if (sent == SOCKET_ERROR) {
            throw TcpConnectionException(
                "Send failed with error: " + std::to_string(WSAGetLastError())
            );
        }

        totalSent += sent;
    }
}

bool TcpConnection::IsValid() const {
    return socket_ != INVALID_SOCKET;
}
