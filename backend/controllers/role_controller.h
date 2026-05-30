#pragma once
#include <drogon/HttpController.h>

class RoleController : public drogon::HttpController<RoleController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(RoleController::request_role, "/api/role/request", drogon::Post);
    ADD_METHOD_TO(RoleController::get_requests, "/api/role/requests", drogon::Get);
    ADD_METHOD_TO(RoleController::approve, "/api/role/requests/{id}/approve", drogon::Post);
    ADD_METHOD_TO(RoleController::reject, "/api/role/requests/{id}/reject", drogon::Post);
    METHOD_LIST_END
    
    void request_role(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_requests(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void approve(const drogon::HttpRequestPtr &req,
                std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void reject(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
};
