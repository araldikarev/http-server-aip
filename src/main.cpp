#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "HttpParser.h"
#include "HttpStructs.h"
#include "Router.h"

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
        return errorCode;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        const int errorCode = WSAGetLastError();
        std::cout << "Listen failed with Winsock error code: " << errorCode << std::endl;
        return errorCode;
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(serverAddr.sin_addr), ipStr, INET_ADDRSTRLEN);

    std::cout << "Bind successfully on " << ipStr << ":" << port << " with code: " << iResult << std::endl;

    bool isRunning = true;
    int i = 0;

    Router router;
    router.AddRoute("GET", "/", [](const HttpRequest& req) {
        HttpResponse res;
        res.headers["Content-Type"] = "text/html; charset=utf-8";
        res.body = "<h1>Welcome to Home Page!</h1>";
        return res;
    });

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
            buffer[bytesReceived] = '\0';

            try {
                HttpRequest parsedReq = HttpParser::Parse(buffer);
                HttpResponse response = router.Route(parsedReq);
                std::string rawResponse = response.to_string();

                send(clientSocket, rawResponse.c_str(), rawResponse.length(), 0);
                std::cout << "Response sent successfully." << std::endl;
            } catch (const HttpParseException& e) {
                std::cout << "Parser error: " << e.what() << std::endl;
                HttpResponse badRequestResponse;
                badRequestResponse.status_code = 400;
                badRequestResponse.status_message = "Bad Request";
                badRequestResponse.body = "<h1>400 Bad Request</h1>";
                std::string rawResponse = badRequestResponse.to_string();
                send(clientSocket, rawResponse.c_str(), rawResponse.length(), 0);
            }
        }
        else if (bytesReceived == 0) {
            std::cout << "Client closed the connection" << std::endl;
        } else {
            std::cout << "Receive failed with error: " << WSAGetLastError() << std::endl;
            continue;
        }
        std::cout << "----------------------------------------\n" << std::endl;
    }

    return 0;
}


int main() {
    try {
        if (initializeWSA() != 0) {
            return 1;
        }

        startTCPSocket();

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
    }

    cleanupWSA();
    return 0;
}
