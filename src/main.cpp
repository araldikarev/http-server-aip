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

int startTCPSocket() {
    const SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listenSocket == INVALID_SOCKET) {
        std::cout << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        cleanupWSA();
        return 1;
    }

    constexpr int port = 8080;

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


    system("timeout /t 30");

    std::cout << "Shutting down server..." << std::endl;
    closesocket(listenSocket);
    std::cout << "Shutdown success." << std::endl;
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
