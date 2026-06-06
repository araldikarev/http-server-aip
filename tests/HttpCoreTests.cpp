#include <doctest/doctest.h>

#include <string>

#include "HttpParser.h"
#include "HttpStructs.h"
#include "Router.h"

namespace {
    size_t CountOccurrences(const std::string &text, const std::string &needle) {
        size_t count = 0;
        size_t position = 0;

        while ((position = text.find(needle, position)) != std::string::npos) {
            ++count;
            position += needle.size();
        }

        return count;
    }
}

TEST_CASE("HttpParser parses valid requests and rejects malformed input") {
    SUBCASE("parses a POST request with trimmed headers and body") {
        const std::string rawRequest =
            "POST /echo HTTP/1.1\r\n"
            "Host:   example.com   \r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "hello";

        const HttpRequest request = HttpParser::Parse(rawRequest);

        CHECK(request.method == "POST");
        CHECK(request.path == "/echo");
        CHECK(request.version == "HTTP/1.1");
        CHECK(request.headers.at("Host") == "example.com");
        CHECK(request.headers.at("Content-Type") == "text/plain");
        CHECK(request.body == "hello");
    }

    SUBCASE("throws on empty input") {
        CHECK_THROWS_AS(HttpParser::Parse(""), HttpParseException);
    }

    SUBCASE("throws on unsupported method") {
        const std::string rawRequest =
            "PUT /items HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "\r\n";

        CHECK_THROWS_AS(HttpParser::Parse(rawRequest), HttpParseException);
    }

    SUBCASE("throws on malformed header") {
        const std::string rawRequest =
            "GET / HTTP/1.1\r\n"
            "BrokenHeader\r\n"
            "\r\n";

        CHECK_THROWS_AS(HttpParser::Parse(rawRequest), HttpParseException);
    }
}

TEST_CASE("Router resolves registered routes and returns 404 for missing paths") {
    SUBCASE("returns the response from a registered handler") {
        Router router;
        router.AddRoute("GET", "/health", [](const HttpRequest &) {
            HttpResponse response;
            response.status_code = 200;
            response.status_message = "OK";
            response.body = "healthy";
            return response;
        });

        HttpRequest request;
        request.method = "GET";
        request.path = "/health";

        const HttpResponse response = router.Route(request);

        CHECK(response.status_code == 200);
        CHECK(response.status_message == "OK");
        CHECK(response.body == "healthy");
    }

    SUBCASE("replaces an existing handler for the same method and path") {
        Router router;
        router.AddRoute("GET", "/config", [](const HttpRequest &) {
            HttpResponse response;
            response.body = "old";
            return response;
        });
        router.AddRoute("GET", "/config", [](const HttpRequest &) {
            HttpResponse response;
            response.body = "new";
            return response;
        });

        HttpRequest request;
        request.method = "GET";
        request.path = "/config";

        const HttpResponse response = router.Route(request);

        CHECK(response.body == "new");
    }

    SUBCASE("returns a generated 404 response for an unknown route") {
        Router router;
        HttpRequest request;
        request.method = "GET";
        request.path = "/missing";

        const HttpResponse response = router.Route(request);

        CHECK(response.status_code == 404);
        CHECK(response.status_message == "Not Found");
        CHECK(response.headers.at("Content-Type") == "text/html; charset=utf-8");
        CHECK(response.body.find("404") != std::string::npos);
    }
}

TEST_CASE("HttpResponse serializes headers and body consistently") {
    SUBCASE("writes the status line, content length and body") {
        HttpResponse response;
        response.status_code = 201;
        response.status_message = "Created";
        response.headers["Content-Type"] = "text/plain; charset=utf-8";
        response.body = "done";

        const std::string rawResponse = response.to_string();

        CHECK(rawResponse.rfind("HTTP/1.1 201 Created\r\n", 0) == 0);
        CHECK(rawResponse.find("Content-Length: 4\r\n") != std::string::npos);
        CHECK(rawResponse.find("Content-Type: text/plain; charset=utf-8\r\n") != std::string::npos);
        CHECK(rawResponse.substr(rawResponse.size() - response.body.size()) == response.body);
    }

    SUBCASE("ignores user supplied Content-Length and uses the real body size") {
        HttpResponse response;
        response.headers["Content-Length"] = "999";
        response.body = "abc";

        const std::string rawResponse = response.to_string();

        CHECK(CountOccurrences(rawResponse, "Content-Length:") == 1);
        CHECK(rawResponse.find("Content-Length: 3\r\n") != std::string::npos);
        CHECK(rawResponse.find("Content-Length: 999\r\n") == std::string::npos);
    }
}
