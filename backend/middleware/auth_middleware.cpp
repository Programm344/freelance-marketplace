#include "auth_middleware.h"

void AuthMiddleware::invoke(const drogon::HttpRequestPtr& req,
                            drogon::MiddlewareNextCallback&& nextCb,
                            drogon::MiddlewareCallback&& mcb) {
    // Извлекаем токен из заголовка Authorization
    std::string authHeader = req->getHeader("Authorization");
    
    if (!authHeader.empty() && authHeader.substr(0, 7) == "Bearer ") {
        std::string token = authHeader.substr(7);
        int userId;
        std::string role;
        
        if (JwtHelper::verifyToken(token, userId, role)) {
            // Сохраняем user_id в атрибутах запроса
            req->attributes()->insert("user_id", userId);
            req->attributes()->insert("role", role);
        }
    }
    
    nextCb(std::move(mcb));
}
