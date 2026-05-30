#pragma once
#include <drogon/HttpController.h>

class OrderController : public drogon::HttpController<OrderController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(OrderController::get_all, "/api/orders", drogon::Get);
    ADD_METHOD_TO(OrderController::get_my_orders, "/api/orders/my", drogon::Get);
    ADD_METHOD_TO(OrderController::get_by_id, "/api/orders/{id}", drogon::Get);
    ADD_METHOD_TO(OrderController::create, "/api/orders", drogon::Post);
    ADD_METHOD_TO(OrderController::update, "/api/orders/{id}", drogon::Put);
    ADD_METHOD_TO(OrderController::delete_order, "/api/orders/{id}", drogon::Delete);
    ADD_METHOD_TO(OrderController::submit_to_moderation, "/api/orders/{id}/submit", drogon::Post);
    ADD_METHOD_TO(OrderController::search, "/api/orders/search", drogon::Get);
    METHOD_LIST_END
    
    void get_all(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_my_orders(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_by_id(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void create(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void update(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void delete_order(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void submit_to_moderation(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void search(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
