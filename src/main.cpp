#include <iostream>
#include "HttpServer.h"

int main() {
    try {
        constexpr int PORT = 8010;
        HttpServer server(PORT);

        server.AddRoute("GET", "/", [](const HttpRequest &request) {
            HttpResponse response;
            response.headers["Content-Type"] = "text/html; charset=utf-8";
            response.body = "<h1>Welcome to Home Page!</h1>";
            return response;
        });

        server.AddRoute("GET", "/about", [](const HttpRequest &request) {
            HttpResponse response;
            response.headers["Content-Type"] = "text/html; charset=utf-8";
            response.body = "<h1>About Page</h1><p>Low-level C++ HTTP Server</p>";
            return response;
        });

        server.AddRoute("POST", "/echo", [](const HttpRequest &request) {
            HttpResponse response;
            response.headers["Content-Type"] = "text/plain; charset=utf-8";
            response.body = "Received body:\n" + request.body;
            return response;
        });

        server.Start();
    } catch (const std::exception &e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
