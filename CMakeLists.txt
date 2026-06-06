cmake_minimum_required(VERSION 3.21)
project(HttpServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(http_server_core
    src/HttpServer.cpp
    src/HttpServer.h
    src/HttpParser.cpp
    src/HttpParser.h
    src/HttpStructs.h
    src/Logger.cpp
    src/Logger.h
    src/Router.cpp
    src/Router.h
    src/TcpConnection.cpp
    src/TcpConnection.h
    src/TcpServer.cpp
    src/TcpServer.h
)

target_include_directories(http_server_core PUBLIC src)

if(WIN32)
    target_link_libraries(http_server_core PUBLIC ws2_32)
endif()

add_executable(HttpServer src/main.cpp)
target_link_libraries(HttpServer PRIVATE http_server_core)

if(BUILD_TESTING)
    add_library(doctest INTERFACE)
    target_include_directories(doctest INTERFACE external)

    add_executable(HttpServerTests
        tests/test_main.cpp
        tests/HttpCoreTests.cpp
    )

    target_link_libraries(HttpServerTests PRIVATE doctest http_server_core)

    add_test(NAME HttpServerTests COMMAND HttpServerTests)
endif()

if(MINGW)
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    set(MINGW_RUNTIME_DLLS
            "${MINGW_BIN_DIR}/libstdc++-6.dll"
            "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
            "${MINGW_BIN_DIR}/libwinpthread-1.dll"
    )

    foreach(runtime_dll IN LISTS MINGW_RUNTIME_DLLS)
        if(EXISTS "${runtime_dll}")
            file(COPY "${runtime_dll}" DESTINATION "${CMAKE_BINARY_DIR}")
        endif()
    endforeach()
endif()
