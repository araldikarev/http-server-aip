#pragma once

#include <winsock2.h>

#include <functional>
#include <stdexcept>
#include <string>

#include "TcpConnection.h"

/// @brief Exception thrown when the TCP server cannot be initialized or operated.
class TcpServerException : public std::runtime_error {
public:
    /// @brief Creates a TCP server exception.
    /// @param message Error description.
    explicit TcpServerException(const std::string &message) : std::runtime_error(message) {}
};

/// @brief TCP server built on top of WinSock2.
class TcpServer {
public:
    /// @brief Callback that processes one accepted client connection.
    using ConnectionHandler = std::function<void(TcpConnection &)>;

private:
    int port_;                             ///< Listening TCP port.
    ConnectionHandler connectionHandler_;  ///< Callback invoked for each client.
    bool isRunning_;                       ///< Indicates whether the accept loop should continue.
    SOCKET listenSocket_;                  ///< Listening socket handle.

public:
    /// @brief Initializes WinSock and stores server settings.
    /// @param port Listening TCP port.
    /// @param connectionHandler Callback for accepted connections.
    TcpServer(int port, ConnectionHandler connectionHandler);

    /// @brief Releases the listening socket and WinSock resources.
    ~TcpServer();

    /// @brief Binds the listening socket and starts the accept loop.
    /// @throws TcpServerException If socket creation, bind, or listen fails.
    void Start();

    /// @brief Stops the accept loop after the current iteration.
    void Stop();

    /// @brief Reports whether the server is currently running.
    /// @return True when the accept loop is active, otherwise false.
    bool IsRunning() const;

    TcpServer(const TcpServer &) = delete;

    TcpServer &operator=(const TcpServer &) = delete;
};
