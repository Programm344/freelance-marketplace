#pragma once
#include <drogon/HttpController.h>

class ResponseController : public drogon::HttpController<ResponseController> {
public:
    METHOD_LIST_BEGIN
    // Отправить отклик на заказ
    ADD_METHOD_TO(ResponseController::create, "/api/responses", drogon::Post);
    // Получить все отклики на заказ (для заказчика)
    ADD_METHOD_TO(ResponseController::get_by_order, "/api/orders/{order_id}/responses", drogon::Get);
    // Получить отклики фрилансера
    ADD_METHOD_TO(ResponseController::get_my_responses, "/api/responses/my", drogon::Get);
    // Принять отклик
    ADD_METHOD_TO(ResponseController::accept, "/api/responses/{id}/accept", drogon::Post);
    // Отклонить отклик
    ADD_METHOD_TO(ResponseController::reject, "/api/responses/{id}/reject", drogon::Post);
    // Отозвать отклик
    ADD_METHOD_TO(ResponseController::withdraw, "/api/responses/{id}/withdraw", drogon::Post);
    METHOD_LIST_END
    
    void create(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_by_order(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback, 
                     int order_id);
    void get_my_responses(const drogon::HttpRequestPtr &req,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void accept(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void reject(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void withdraw(const drogon::HttpRequestPtr &req,
                 std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
};
