#pragma once
#include <functional>
#include "HttpStructs.h"

using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief Класс маршрутизатора (Router) для сопоставления путей и обработчиков.
 */
class Router {
public:
    /**
     * @brief Добавляет новый маршрут в таблицу маршрутизации.
     *
     * @param method HTTP метод (например, "GET", "POST").
     * @param path URL путь (например, "/").
     * @param handler Функция-обработчик для данного маршрута.
     */
    void AddRoute(const std::string& method, const std::string& path, const HttpHandler &handler);

    /**
     * @brief Ищет подходящий обработчик для запроса и возвращает ответ.
     *
     * @param request Входящий распарсенный HTTP-запрос.
     * @return Объект HttpResponse.
     */
    HttpResponse Route(const HttpRequest& request) const;
private:
    std::unordered_map<std::string, HttpHandler> routes_;
};