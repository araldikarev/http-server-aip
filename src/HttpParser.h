#pragma once
#include <stdexcept>
#include "HttpStructs.h"


/**
 * @brief Исключение, выбрасываемое при ошибках парсинга HTTP-запроса.
 */
class HttpParseException : public std::runtime_error {
public:
    explicit HttpParseException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Класс для парсинга сырых текстовых HTTP-запросов.
 */
class HttpParser {
public:
    /**
     * @brief Парсит сырой HTTP-запрос из строки в структуру HttpRequest.
     *
     * @param rawRequest Входящая строка с сырым HTTP-запросом.
     * @return Заполненная структура HttpRequest.
     * @throws HttpParseException Если запрос имеет некорректный формат.
     */
    static HttpRequest Parse(const std::string &rawRequest);
};

