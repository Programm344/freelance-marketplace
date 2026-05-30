#pragma once
#include <drogon/HttpRequest.h>
#include "../utils/jwt_helper.h"

inline int getUserIdFromRequest(const drogon::HttpRequestPtr& req) {
    // Пробуем получить из атрибутов (если middleware отработал)
    auto attrs = req->getAttributes();
    if (attrs && attrs->find("user_id") != attrs->end()) {
        return attrs->get<int>("user_id");
    }
    
    // Fallback: пробуем из заголовка Authorization
    std::string auth = req->getHeader("Authorization");
    if (!auth.empty() && auth.substr(0, 7) == "Bearer ") {
        std::string token = auth.substr(7);
        int userId;
        std::string role;
        if (JwtHelper::verifyToken(token, userId, role)) {
            return userId;
        }
    }
    
    return 1; // fallback для обратной совместимости
}
