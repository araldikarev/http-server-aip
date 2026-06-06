#include "TcpServer.h"

#include <ws2tcpip.h>

#include <utility>

#include "Logger.h"

TcpServer::TcpServer(int port, ConnectionHandler connectionHandler)
    : port_(port), connectionHandler_(std::move(connectionHandler)), isRunning_(false), listenSocket_(INVALID_SOCKET) {
    WSAData data;
    if (const int iResult = WSAStartup(MAKEWORD(2, 2), &data); iResult != 0) {
        throw TcpServerException("WSAStartup failed with error: " + std::to_string(iResult));
    }
    Logger::Info("WinSock initialized successfully.");
}

TcpServer::~TcpServer() {
    if (listenSocket_ != INVALID_SOCKET) {
        closesocket(listenSocket_);
    }
    WSACleanup();
    Logger::Info("WinSock cleaned up.");
}

void TcpServer::Stop() {
    isRunning_ = false;
}

void TcpServer::Start() {
    listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket_ == INVALID_SOCKET) {
        throw TcpServerException("Socket creation failed with error: " + std::to_string(WSAGetLastError()));
    }

    SOCKADDR_IN serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    const int iResult = bind(listenSocket_, reinterpret_cast<SOCKADDR *>(&serverAddr), sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        const int errorCode = WSAGetLastError();
        throw TcpServerException("Bind failed with code: " + std::to_string(errorCode));
    }

    if (listen(listenSocket_, SOMAXCONN) == SOCKET_ERROR) {
        const int errorCode = WSAGetLastError();
        throw TcpServerException("Listen failed with error code: " + std::to_string(errorCode));
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
    Logger::Info("Server successfully listening on " + std::string(ipStr) + ":" + std::to_string(port_));
    isRunning_ = true;

    while (isRunning_) {
        SOCKADDR_IN clientAddr{};
        int clientAddrSize = sizeof(clientAddr);

        SOCKET clientSocketRaw = accept(listenSocket_, reinterpret_cast<SOCKADDR *>(&clientAddr), &clientAddrSize);
        if (clientSocketRaw == INVALID_SOCKET) {
            Logger::Error("Accept failed with error: " + std::to_string(WSAGetLastError()));
            continue;
        }

        char ipClientStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipClientStr, INET_ADDRSTRLEN);
        Logger::Info("Client connected: " + std::string(ipClientStr) + ":" +
                     std::to_string(ntohs(clientAddr.sin_port)));

        try {
            TcpConnection connection(clientSocketRaw);
            connectionHandler_(connection);
            Logger::Info("Client handled successfully.");
        } catch (const std::exception &e) {
            Logger::Error("Client handling failed: " + std::string(e.what()));
        }
    }
}

bool TcpServer::IsRunning() const {
    return isRunning_;
}
