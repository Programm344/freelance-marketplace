#pragma once
#include <drogon/HttpController.h>

class MessageController : public drogon::HttpController<MessageController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MessageController::send, "/api/messages", drogon::Post);
    ADD_METHOD_TO(MessageController::get_by_order, "/api/orders/{order_id}/messages", drogon::Get);
    ADD_METHOD_TO(MessageController::get_my_conversations, "/api/messages/conversations", drogon::Get);
    METHOD_LIST_END
    
    void send(const drogon::HttpRequestPtr &req,
             std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_by_order(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback, int order_id);
    void get_my_conversations(const drogon::HttpRequestPtr &req,
                             std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
