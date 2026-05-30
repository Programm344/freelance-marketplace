#pragma once
#include <drogon/HttpController.h>

class NotificationController : public drogon::HttpController<NotificationController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(NotificationController::get_my, "/api/notifications", drogon::Get);
    ADD_METHOD_TO(NotificationController::mark_read, "/api/notifications/{id}/read", drogon::Post);
    ADD_METHOD_TO(NotificationController::mark_all_read, "/api/notifications/read-all", drogon::Post);
    ADD_METHOD_TO(NotificationController::get_stats, "/api/reports/stats", drogon::Get);
    METHOD_LIST_END
    
    void get_my(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void mark_read(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void mark_all_read(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_stats(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
