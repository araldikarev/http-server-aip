#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")


int initializeWSA() {
    WSADATA wsadata;

    const int iResult = WSAStartup(MAKEWORD(2,2), &wsadata);

    if (iResult != 0) {
        std::cout << "WSAStartup failed with error code: " << iResult << std::endl;
        return iResult;
    }

    std::cout << "WSAStartup started successfully with code: " << iResult << std::endl;
    std::cout << "wsadata szDescription: " << wsadata.szDescription << std::endl;
    std::cout << "wsadata szSystemStatus: " << wsadata.szSystemStatus << std::endl;
    std::cout << "wsadata wHighVersion: " << wsadata.wHighVersion << std::endl;
    std::cout << "wsadata wVersion: " << wsadata.wVersion << std::endl;
    return iResult;
}

int cleanupWSA() {
    const int iResult = WSACleanup();
    return iResult;
}

struct SocketGuard {
    SOCKET socket;
    explicit SocketGuard(SOCKET s) : socket(s) {}
    ~SocketGuard() {if (socket != INVALID_SOCKET) closesocket(socket);}

    operator SOCKET() const { return socket; }

    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;
};

int startTCPSocket() {
    const SOCKET listenSocketRaw = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listenSocketRaw == INVALID_SOCKET) {
        std::cout << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }
    const SocketGuard listenSocket(listenSocketRaw);

    constexpr int port = 8010;

    SOCKADDR_IN serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    const auto *socketAddress = reinterpret_cast<SOCKADDR*>(&serverAddr);

    const int iResult = bind(listenSocket, socketAddress, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        const int errorCode = WSAGetLastError();
        std::cout << "Bind failed with code: " << errorCode << std::endl;
        closesocket(listenSocket);
        return errorCode;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        const int errorCode = WSAGetLastError();
        std::cout << "Listen failed with Winsock error code: " << errorCode << std::endl;
        closesocket(listenSocket);
        return errorCode;
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddr.sin_addr), ipStr, INET_ADDRSTRLEN);

    std::cout << "Bind successfully on " << ipStr << ":" << port << " with code: " << iResult << std::endl;

    bool isRunning = true;
    int i = 0;
    while (isRunning) {
        SOCKADDR_IN clientAddr{};
        int clientAddrSize = sizeof(clientAddr);

        const SOCKET clientSocketRaw = accept(listenSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &clientAddrSize);
        if (clientSocketRaw == INVALID_SOCKET) {
            std::cout << "Accept failed with error: " << WSAGetLastError() << std::endl;
            continue;
        }
        const SocketGuard clientSocket(clientSocketRaw);

        char ipClientStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipClientStr, INET_ADDRSTRLEN);

        std::cout << "Client " << ipClientStr << ":" << ntohs(clientAddr.sin_port) << " successfully accepted." << std::endl;

        char buffer[2048];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived > 0) {
            std::cout << "Got " << bytesReceived << " bytes from client" << std::endl;
            buffer[bytesReceived] = '\0';

            std::cout << buffer << std::endl;

            const std::string body = "<html><body><h1>Hello from C++ HTTP Server!</h1></body></html>";

            std::string protocolCode = "HTTP/1.1 200 OK\r\n";
            std::string contentType = "Content-Type: text/html; charset=utf-8\r\n";
            std::string contentLength = "Content-Length: " + std::to_string(body.length()) + "\r\n";
            std::string connectionType = "Connection: close\r\n";

            std::string fullResponse = protocolCode + contentType + contentLength + connectionType + "\r\n" + body;


            const char* content = fullResponse.c_str();
            const int responseLength = fullResponse.length();
            int bytesSent = send(clientSocket, content, responseLength, 0);
            if (bytesSent == responseLength) {
                std::cout << "Response has sent successfully with " << responseLength << " bytes." << std::endl;
            }
            else if (bytesSent >= 0) {
                std::cout << "Sent " << bytesSent << " bytes to client, lost " << responseLength-bytesSent << " bytes." << std::endl;
            }
            else {
                std::cout << "Send error: " << WSAGetLastError() << std::endl;
            }
        }
        else if (bytesReceived == 0) {
            std::cout << "Client closed the connection" << std::endl;
        } else {
            std::cout << "Receive failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            continue;
        }
        i  += 1;
        if (i > 9) isRunning = false;
        closesocket(clientSocket);
        std::cout << "----------------------------------------\n" << std::endl;
    }

    closesocket(listenSocket);
    return 0;
}


int main() {
    //Инициализация сетевого
    if (initializeWSA() != 0) {
        return 1;
    }

    if (const int tcpResult = startTCPSocket(); tcpResult != 0) {
        std::cout << "TCP Socket failed with code: " << tcpResult << std::endl;
    }

    cleanupWSA();
    return 0;
}
