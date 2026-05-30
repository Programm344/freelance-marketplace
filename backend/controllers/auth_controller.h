#pragma once
#include <drogon/HttpController.h>
#include "../services/auth_service.h"

class AuthController : public drogon::HttpController<AuthController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthController::register_user, "/api/auth/register", drogon::Post);
    ADD_METHOD_TO(AuthController::login, "/api/auth/login", drogon::Post);
    ADD_METHOD_TO(AuthController::test, "/api/test", drogon::Get);
    METHOD_LIST_END
    
    void register_user(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void login(const drogon::HttpRequestPtr &req,
              std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void test(const drogon::HttpRequestPtr &req,
             std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    
private:
    AuthService authService_;
};
