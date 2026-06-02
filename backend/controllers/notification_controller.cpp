#include "notification_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include "../utils/request_helper.h"

using namespace drogon;

void NotificationController::get_my(const HttpRequestPtr &req,
                                    std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        int userId = getUserIdFromRequest(req);
        auto result = dbClient->execSqlSync(
            "SELECT * FROM notifications WHERE user_id = $1 ORDER BY created_at DESC LIMIT 50", userId
        );
        Json::Value notifications(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value n;
            n["id"] = row["id"].as<int>();
            n["type"] = row["type"].as<std::string>();
            n["title"] = row["title"].as<std::string>();
            n["message"] = row["message"].as<std::string>();
            n["is_read"] = row["is_read"].as<bool>();
            n["created_at"] = row["created_at"].as<std::string>();
            notifications.append(n);
        }
        Json::Value response;
        response["status"] = "success";
        response["notifications"] = notifications;
        auto resp = HttpResponse::newHttpJsonResponse(response); callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void NotificationController::mark_read(const HttpRequestPtr &req,
                                       std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto dbClient = app().getDbClient("default");
        dbClient->execSqlSync("UPDATE notifications SET is_read = true WHERE id = $1", id);
        Json::Value response; response["status"] = "success";
        auto resp = HttpResponse::newHttpJsonResponse(response); callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void NotificationController::mark_all_read(const HttpRequestPtr &req,
                                           std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        int userId = getUserIdFromRequest(req);
        dbClient->execSqlSync("UPDATE notifications SET is_read = true WHERE user_id = $1", userId);
        Json::Value response; response["status"] = "success";
        auto resp = HttpResponse::newHttpJsonResponse(response); callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void NotificationController::get_stats(const HttpRequestPtr &req,
                                       std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        auto stats = dbClient->execSqlSync(
            "SELECT (SELECT COUNT(*) FROM orders WHERE status='published') as pub,"
            "(SELECT COUNT(*) FROM orders WHERE status='in_progress') as prog,"
            "(SELECT COUNT(*) FROM orders WHERE status='completed') as comp,"
            "(SELECT COUNT(*) FROM responses) as resp,"
            "(SELECT COUNT(*) FROM users) as users"
        );
        Json::Value response; response["status"] = "success";
        Json::Value s;
        if (stats.size() > 0) {
            s["published"] = stats[0]["pub"].as<int>();
            s["in_progress"] = stats[0]["prog"].as<int>();
            s["completed"] = stats[0]["comp"].as<int>();
            s["responses"] = stats[0]["resp"].as<int>();
            s["users"] = stats[0]["users"].as<int>();
        }
        response["stats"] = s;
        auto resp = HttpResponse::newHttpJsonResponse(response); callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}
