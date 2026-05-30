#pragma once
#include <drogon/HttpController.h>

class ModerationController : public drogon::HttpController<ModerationController> {
public:
    METHOD_LIST_BEGIN
    // Список заказов на модерации
    ADD_METHOD_TO(ModerationController::get_queue, "/api/moderation/orders", drogon::Get);
    // Одобрить заказ
    ADD_METHOD_TO(ModerationController::approve, "/api/moderation/orders/{id}/approve", drogon::Post);
    // Отклонить заказ
    ADD_METHOD_TO(ModerationController::reject, "/api/moderation/orders/{id}/reject", drogon::Post);
    // Отправить на доработку
    ADD_METHOD_TO(ModerationController::request_revision, "/api/moderation/orders/{id}/revise", drogon::Post);
    // Список жалоб
    ADD_METHOD_TO(ModerationController::get_complaints, "/api/moderation/complaints", drogon::Get);
    // Рассмотреть жалобу
    ADD_METHOD_TO(ModerationController::resolve_complaint, "/api/moderation/complaints/{id}/resolve", drogon::Post);
    // Проверить внешние заказы
    ADD_METHOD_TO(ModerationController::check_external_orders, "/api/moderation/external-orders", drogon::Get);
    METHOD_LIST_END
    
    void get_queue(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void approve(const drogon::HttpRequestPtr &req,
                std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void reject(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void request_revision(const drogon::HttpRequestPtr &req,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void get_complaints(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void resolve_complaint(const drogon::HttpRequestPtr &req,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback, int id);
    void check_external_orders(const drogon::HttpRequestPtr &req,
                              std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
