#pragma once
#include <drogon/HttpController.h>

class UserController : public drogon::HttpController<UserController> {
public:
    METHOD_LIST_BEGIN
    // Профиль текущего пользователя
    ADD_METHOD_TO(UserController::get_profile, "/api/user/profile", drogon::Get);
    ADD_METHOD_TO(UserController::update_profile, "/api/user/profile", drogon::Put);
    
    // Публичный профиль фрилансера
    ADD_METHOD_TO(UserController::get_freelancer_profile, "/api/freelancers/{id}", drogon::Get);
    
    // Публичный профиль заказчика
    ADD_METHOD_TO(UserController::get_customer_profile, "/api/customers/{id}", drogon::Get);
    
    // Список фрилансеров с фильтрацией
    ADD_METHOD_TO(UserController::search_freelancers, "/api/freelancers/search", drogon::Get);
    METHOD_LIST_END
    
    void get_profile(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void update_profile(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_freelancer_profile(const drogon::HttpRequestPtr &req,
                               std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                               int id);
    void get_customer_profile(const drogon::HttpRequestPtr &req,
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                             int id);
    void search_freelancers(const drogon::HttpRequestPtr &req,
                           std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
