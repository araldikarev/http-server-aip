cmake_minimum_required(VERSION 4.2)
project(HttpServer)

set(CMAKE_CXX_STANDARD 26)

add_executable(HttpServer src/main.cpp
        src/HttpStructs.h
        src/TcpServer.cpp
        src/TcpServer.h
        src/HttpParser.cpp
        src/HttpParser.h
        src/Router.cpp
        src/Router.h
        src/Logger.cpp
        src/Logger.h
        src/HttpServer.cpp
        src/HttpServer.h
)

if(WIN32)
    target_link_libraries(HttpServer ws2_32)
endif()