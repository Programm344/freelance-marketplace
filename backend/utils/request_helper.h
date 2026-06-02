#pragma once
#include <drogon/HttpRequest.h>
#include "../utils/jwt_helper.h"

inline int getUserIdFromRequest(const drogon::HttpRequestPtr& req) {
    std::string auth = req->getHeader("Authorization");
    if (!auth.empty() && auth.substr(0, 7) == "Bearer ") {
        std::string token = auth.substr(7);
        int userId;
        std::string role;
        if (JwtHelper::verifyToken(token, userId, role)) {
            return userId;
        }
    }
    return 0; // незалогинен — вернёт 0, операции не пройдут
}
