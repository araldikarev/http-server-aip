#pragma once

#include <functional>
#include <unordered_map>

#include "HttpStructs.h"

/// @brief Request handler used by the router.
using HttpHandler = std::function<HttpResponse(const HttpRequest &)>;

/// @brief Matches HTTP method/path pairs to request handlers.
class Router {
private:
    std::unordered_map<std::string, HttpHandler> routes_;  ///< Registered routes keyed by "METHOD path".

public:
    /// @brief Registers or replaces a handler for the given method/path pair.
    /// @param method HTTP method such as "GET" or "POST".
    /// @param path URL path such as "/" or "/about".
    /// @param handler Handler that builds the response for the route.
    void AddRoute(const std::string &method, const std::string &path, const HttpHandler &handler);

    /// @brief Resolves a request to a handler and returns the produced response.
    /// @param request Parsed HTTP request.
    /// @return Handler result or a generated 404 response when no route matches.
    HttpResponse Route(const HttpRequest &request) const;
};
