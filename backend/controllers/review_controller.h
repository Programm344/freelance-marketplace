#pragma once
#include <drogon/HttpController.h>

class ReviewController : public drogon::HttpController<ReviewController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ReviewController::create, "/api/reviews", drogon::Post);
    ADD_METHOD_TO(ReviewController::get_for_user, "/api/users/{id}/reviews", drogon::Get);
    METHOD_LIST_END
    
    void create(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_for_user(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
};
