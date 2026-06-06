#pragma once

#include <winsock2.h>

#include <stdexcept>
#include <string>

/// @brief Exception thrown when a socket send or receive operation fails.
class TcpConnectionException : public std::runtime_error {
public:
    /// @brief Creates a TCP connection exception.
    /// @param message Error description.
    explicit TcpConnectionException(const std::string &message) : std::runtime_error(message) {}
};

/// @brief RAII wrapper around a connected WinSock socket.
class TcpConnection {
private:
    SOCKET socket_;  ///< Owned socket handle.

public:
    /// @brief Takes ownership of an existing socket handle.
    /// @param socket Connected socket handle.
    explicit TcpConnection(SOCKET socket);

    /// @brief Closes the owned socket if it is valid.
    ~TcpConnection();

    TcpConnection(const TcpConnection &) = delete;

    TcpConnection &operator=(const TcpConnection &) = delete;

    /// @brief Transfers socket ownership from another connection object.
    /// @param other Source object that relinquishes ownership.
    TcpConnection(TcpConnection &&other) noexcept;

    /// @brief Releases the current socket and takes ownership from another object.
    /// @param other Source object that relinquishes ownership.
    /// @return Reference to the current object.
    TcpConnection &operator=(TcpConnection &&other) noexcept;

    /// @brief Receives one chunk of bytes from the socket.
    /// @param bufferSize Size of the temporary receive buffer in bytes.
    /// @return Received bytes converted to a string.
    /// @throws TcpConnectionException If the client closes the connection or recv fails.
    std::string ReceiveChunk(size_t bufferSize = 2048) const;

    /// @brief Sends the entire message over the socket.
    /// @param message Raw bytes to send.
    /// @throws TcpConnectionException If send fails before all bytes are transferred.
    void SendAll(const std::string &message) const;

    /// @brief Checks whether the object currently owns a valid socket.
    /// @return True when the socket handle is valid, otherwise false.
    bool IsValid() const;
};
