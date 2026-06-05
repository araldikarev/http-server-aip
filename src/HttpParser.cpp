#include "HttpParser.h"

std::string_view  Trim(std::string_view str) {
    while (!str.empty() && std::isspace(static_cast<unsigned char>(str.front()))) {
        str.remove_prefix(1);
    }
    while (!str.empty() && std::isspace(static_cast<unsigned char>(str.back()))) {
        str.remove_suffix(1);
    }
    return str;
}

HttpRequest HttpParser::Parse(const std::string &rawRequest) {
    if (rawRequest.empty()) {
        throw HttpParseException("Empty request received");
    }

    HttpRequest request;
    std::string_view view(rawRequest);

    const size_t method_end = view.find(' ');
    if (method_end == std::string_view::npos) {
        throw HttpParseException("Malformed request: Method not found");
    }
    request.method = view.substr(0, method_end);
    view.remove_prefix(method_end+1);

    const size_t path_end = view.find(' ');
    if (path_end == std::string_view::npos) {
        throw HttpParseException("Malformed request: Path not found");
    }
    request.path = view.substr(0, path_end);
    view.remove_prefix(path_end+1);

    const size_t version_end = view.find('\r');
    if (version_end == std::string_view::npos) {
        throw HttpParseException("Malformed request: HTTP version end not found");
    }
    request.version = view.substr(0, version_end);
    view.remove_prefix(version_end+2);

    while (true) {

        const size_t header_end = view.find("\r\n");
        if (header_end == std::string_view::npos) {
            break;
        }

        std::string_view header = view.substr(0, header_end);

        if (header.empty()) {
            view.remove_prefix(2);
            break;
        }

        if (const size_t colon_pos  = header.find(":"); colon_pos != std::string_view::npos) {
            std::string_view header_name = Trim(header.substr(0, colon_pos));
            std::string_view header_value = Trim(header.substr(colon_pos + 1));

            request.headers.insert_or_assign(std::string(header_name), std::string(header_value));
        } else {
            throw HttpParseException("Malformed header: missing colon");
        }

        view.remove_prefix(header_end + 2);
    }

    request.body = view;

    return request;
}
