#include "message_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include "../utils/request_helper.h"

using namespace drogon;

void MessageController::send(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    try {
        auto dbClient = app().getDbClient("default");
        int userId = getUserIdFromRequest(req);
        int orderId = (*json)["order_id"].asInt();
        
        // Проверяем доступ к чату
        auto access = dbClient->execSqlSync(
            "SELECT customer_id FROM orders WHERE id = $1", orderId
        );
        auto accepted = dbClient->execSqlSync(
            "SELECT freelancer_id FROM responses WHERE order_id = $1 AND status = 'accepted'", orderId
        );
        bool hasAccess = false;
        if (access.size() > 0 && access[0]["customer_id"].as<int>() == userId) hasAccess = true;
        if (accepted.size() > 0 && accepted[0]["freelancer_id"].as<int>() == userId) hasAccess = true;
        if (!hasAccess) {
            Json::Value err; err["error"] = "Нет доступа к этому чату";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k403Forbidden); callback(resp); return;
        }
        
        auto result = dbClient->execSqlSync(
            "INSERT INTO messages (order_id, sender_id, message) "
            "VALUES ($1, $2, $3) RETURNING id, created_at",
            orderId, userId, (*json)["message"].asString()
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message_id"] = result[0]["id"].as<int>();
        response["created_at"] = result[0]["created_at"].as<std::string>();
        auto resp = HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(k201Created); callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void MessageController::get_by_order(const HttpRequestPtr &req,
                                     std::function<void(const HttpResponsePtr &)> &&callback, int order_id) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT m.*, u.email as sender_email "
            "FROM messages m JOIN users u ON m.sender_id = u.id "
            "WHERE m.order_id = $1 ORDER BY m.created_at ASC", order_id
        );
        Json::Value messages(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value msg;
            msg["id"] = row["id"].as<int>();
            msg["sender_email"] = row["sender_email"].as<std::string>();
            msg["sender_id"] = row["sender_id"].as<int>();
            msg["message"] = row["message"].as<std::string>();
            msg["created_at"] = row["created_at"].as<std::string>();
            messages.append(msg);
        }
        Json::Value response;
        response["status"] = "success";
        response["messages"] = messages;
        auto resp = HttpResponse::newHttpJsonResponse(response); callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void MessageController::get_my_conversations(const HttpRequestPtr &req,
                                              std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        int userId = getUserIdFromRequest(req);
        auto result = dbClient->execSqlSync(
            "SELECT DISTINCT o.id as order_id, o.title as order_title, o.status as order_status, "
            "(SELECT message FROM messages WHERE order_id = o.id ORDER BY created_at DESC LIMIT 1) as last_message, "
            "(SELECT created_at FROM messages WHERE order_id = o.id ORDER BY created_at DESC LIMIT 1) as last_time "
            "FROM orders o JOIN messages m ON o.id = m.order_id "
            "WHERE o.customer_id = $1 OR o.id IN "
            "(SELECT order_id FROM responses WHERE freelancer_id = $1 AND status = 'accepted') "
            "ORDER BY last_time DESC", userId
        );
        Json::Value conversations(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value conv;
            conv["order_id"] = row["order_id"].as<int>();
            conv["order_title"] = row["order_title"].as<std::string>();
            conv["status"] = row["order_status"].as<std::string>();
            conv["last_message"] = row["last_message"].isNull() ? "" : row["last_message"].as<std::string>();
            conv["last_time"] = row["last_time"].isNull() ? "" : row["last_time"].as<std::string>();
            conversations.append(conv);
        }
        Json::Value response;
        response["status"] = "success";
        response["conversations"] = conversations;
        auto resp = HttpResponse::newHttpJsonResponse(response); callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}
