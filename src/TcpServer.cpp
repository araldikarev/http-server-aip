#include <ws2tcpip.h>
#include <utility>

#include "TcpServer.h"
#include "Logger.h"

struct SocketGuard {
    SOCKET socket;
    explicit SocketGuard(SOCKET s) : socket(s) {}
    ~SocketGuard() {if (socket != INVALID_SOCKET) closesocket(socket);}

    operator SOCKET() const { return socket; }

    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;
};

TcpServer::TcpServer(int port, DataHandler dataHandler)
    : port_(port), dataHandler_(std::move(dataHandler)), isRunning_(false), listenSocket_(INVALID_SOCKET)
{
    WSAData data;
    if (const int iResult = WSAStartup(MAKEWORD(2,2), &data); iResult != 0) {
        throw TcpServerException("WSAStartup failed with error" +  std::to_string(iResult));
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

    const int iResult = bind(listenSocket_, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr));
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
    Logger::Info(
            "Server successfully listening on : " +
            std::string(ipStr) +
            ":" +
            std::to_string(port_));
    isRunning_ = true;
    int requestCount = 0;

    while (isRunning_) {
        SOCKADDR_IN clientAddr{};
        int clientAddrSize = sizeof(clientAddr);

        SOCKET clientSocketRaw = accept(listenSocket_, reinterpret_cast<SOCKADDR*>(&clientAddr), &clientAddrSize);
        if (clientSocketRaw == INVALID_SOCKET) {
            Logger::Error("Accept failed with error: " + std::to_string(WSAGetLastError()));
            continue;
        }
        const SocketGuard clientSocket(clientSocketRaw);

        char ipClientStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipClientStr, INET_ADDRSTRLEN);
        Logger::Info(
            "Client connected: " +
            std::string(ipClientStr) +
            ":" +
            std::to_string(ntohs(clientAddr.sin_port))
        );

        try {
            std::string rawRequest = Receive(clientSocket);
            std::string rawResponse = dataHandler_(rawRequest);

            Send(clientSocket, rawResponse);

            Logger::Info("Response sent successfully.");
        }
        catch (const std::exception& e) {
            Logger::Error("Client handling failed: " + std::string(e.what()));
        }

        requestCount++;
    }
}

bool TcpServer::IsRunning() const {
    return isRunning_;
}

std::string TcpServer::Receive(SOCKET clientSocket) const {
    constexpr size_t BufferSize = 2048;
    constexpr size_t MaxRequestSize = 1024 * 64; // 64 KB

    std::string data;
    char buffer[BufferSize];

    while (data.find("\r\n\r\n") == std::string::npos) {
        const int bytesReceived = recv(clientSocket, buffer,static_cast<int>(sizeof(buffer)),0);

        if (bytesReceived > 0) {
            data.append(buffer, bytesReceived);

            if (data.size() > MaxRequestSize) {
                throw TcpServerException("Request is too large");
            }

            continue;
        }

        if (bytesReceived == 0) {
            throw TcpServerException("Client closed connection cleanly.");
        }

        throw TcpServerException(
            "Receive failed with error: " + std::to_string(WSAGetLastError())
        );
    }

    return data;
}

void TcpServer::Send(SOCKET clientSocket, const std::string& data) const {
    int totalSent = 0;
    const int dataSize = static_cast<int>(data.size());

    while (totalSent < dataSize) {
        const int sent = send(clientSocket, data.c_str() + totalSent, dataSize - totalSent, 0);
        if (sent == SOCKET_ERROR) {
            throw TcpServerException(
                "Send failed with error: " + std::to_string(WSAGetLastError())
            );
        }

        totalSent += sent;
    }
}
