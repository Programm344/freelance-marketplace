#pragma once
#include <drogon/HttpController.h>

class AdminController : public drogon::HttpController<AdminController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AdminController::get_users, "/api/admin/users", drogon::Get);
    ADD_METHOD_TO(AdminController::update_user, "/api/admin/users/{id}", drogon::Put);
    ADD_METHOD_TO(AdminController::block_user, "/api/admin/users/{id}/block", drogon::Post);
    ADD_METHOD_TO(AdminController::unblock_user, "/api/admin/users/{id}/unblock", drogon::Post);
    ADD_METHOD_TO(AdminController::get_stats, "/api/admin/stats", drogon::Get);
    ADD_METHOD_TO(AdminController::export_csv, "/api/admin/export/csv", drogon::Get);
    METHOD_LIST_END
    
    void get_users(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void update_user(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void block_user(const drogon::HttpRequestPtr &req,
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void unblock_user(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void get_stats(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void export_csv(const drogon::HttpRequestPtr &req,
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
