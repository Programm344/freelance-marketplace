#pragma once
#include <drogon/HttpController.h>

class ComplaintController : public drogon::HttpController<ComplaintController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ComplaintController::create, "/api/complaints", drogon::Post);
    ADD_METHOD_TO(ComplaintController::get_my, "/api/complaints/my", drogon::Get);
    METHOD_LIST_END
    
    void create(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_my(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
