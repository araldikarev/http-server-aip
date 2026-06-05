#pragma once
#include "HttpStructs.h"

class HttpParser {
public:
    static HttpRequest Parse(const std::string &rawRequest);
};

