#pragma once
#include <drogon/HttpController.h>

class SearchController : public drogon::HttpController<SearchController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(SearchController::save, "/api/searches", drogon::Post);
    ADD_METHOD_TO(SearchController::get_my, "/api/searches/my", drogon::Get);
    ADD_METHOD_TO(SearchController::remove, "/api/searches/{id}", drogon::Delete);
    METHOD_LIST_END
    
    void save(const drogon::HttpRequestPtr &req,
             std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_my(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void remove(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
};
