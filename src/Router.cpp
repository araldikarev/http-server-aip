#include "Router.h"

void Router::AddRoute(const std::string& method, const std::string& path, const HttpHandler &handler) {
    const std::string key = method + " " + path;
    routes_[key] = handler;
}

HttpResponse Router::Route(const HttpRequest& request) const {
    const std::string key = request.method + " " + request.path;

    auto it = routes_.find(key);
    if (it != routes_.end()) {
        return it->second(request);
    }

    HttpResponse response;
    response.status_code = 404;
    response.status_message = "Not Found";
    response.headers["Content-Type"] = "text/html; charset=utf-8";
    response.body = "<html><body><h1>404 Page Not Found</h1></body></html>";
    return response;
}