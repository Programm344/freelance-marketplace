#pragma once
#include <drogon/HttpMiddleware.h>
#include "../utils/jwt_helper.h"

class AuthMiddleware : public drogon::HttpMiddleware<AuthMiddleware> {
public:
    void invoke(const drogon::HttpRequestPtr& req,
                drogon::MiddlewareNextCallback&& nextCb,
                drogon::MiddlewareCallback&& mcb) override;
};
