#include "notification_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>

using namespace drogon;

void NotificationController::get_my(const HttpRequestPtr &req,
                                    std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "SELECT * FROM notifications WHERE user_id = 1 ORDER BY created_at DESC LIMIT 50"
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
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void NotificationController::mark_read(const HttpRequestPtr &req,
                                       std::function<void(const HttpResponsePtr &)> &&callback,
                                       int id) {
    try {
        auto dbClient = app().getDbClient("default");
        dbClient->execSqlSync("UPDATE notifications SET is_read = true WHERE id = $1", id);
        
        Json::Value response;
        response["status"] = "success";
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void NotificationController::mark_all_read(const HttpRequestPtr &req,
                                           std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        dbClient->execSqlSync("UPDATE notifications SET is_read = true WHERE user_id = 1");
        
        Json::Value response;
        response["status"] = "success";
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void NotificationController::get_stats(const HttpRequestPtr &req,
                                       std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto stats = dbClient->execSqlSync(
            "SELECT "
            "(SELECT COUNT(*) FROM orders WHERE status = 'published') as published_orders, "
            "(SELECT COUNT(*) FROM orders WHERE status = 'on_moderation') as moderation_orders, "
            "(SELECT COUNT(*) FROM orders WHERE status = 'in_progress') as in_progress_orders, "
            "(SELECT COUNT(*) FROM orders WHERE status = 'completed') as completed_orders, "
            "(SELECT COUNT(*) FROM responses) as total_responses, "
            "(SELECT COUNT(*) FROM external_orders WHERE status = 'new') as new_external_orders, "
            "(SELECT COUNT(*) FROM users) as total_users, "
            "(SELECT COUNT(*) FROM crawler_sources WHERE status = 'active') as active_sources"
        );
        
        Json::Value response;
        response["status"] = "success";
        
        if (stats.size() > 0) {
            auto row = stats[0];
            response["stats"]["published_orders"] = row["published_orders"].as<int>();
            response["stats"]["moderation_orders"] = row["moderation_orders"].as<int>();
            response["stats"]["in_progress_orders"] = row["in_progress_orders"].as<int>();
            response["stats"]["completed_orders"] = row["completed_orders"].as<int>();
            response["stats"]["total_responses"] = row["total_responses"].as<int>();
            response["stats"]["new_external_orders"] = row["new_external_orders"].as<int>();
            response["stats"]["total_users"] = row["total_users"].as<int>();
            response["stats"]["active_sources"] = row["active_sources"].as<int>();
        }
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}
