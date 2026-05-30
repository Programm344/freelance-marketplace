#include "../utils/request_helper.h"
#include "search_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include "../utils/helpers.h"

using namespace drogon;

void SearchController::save(const HttpRequestPtr &req,
                            std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    
    try {
        auto dbClient = app().getDbClient("default");
        
        std::string skills = (*json)["skills"].asString();
        
        auto result = dbClient->execSqlSync(
            "INSERT INTO saved_searches (user_id, name, keywords, skills, budget_min, budget_max) "
            "VALUES ($1, $2, $3, $4, $5, $6) RETURNING id",
            1, // TODO: из токена
            (*json)["name"].asString(),
            toPgArray((*json)["keywords"].asString()),
            toPgArray(skills),
            (*json)["budget_min"].asDouble(),
            (*json)["budget_max"].asDouble()
        );
        
        Json::Value response;
        response["status"] = "success";
        response["search_id"] = result[0]["id"].as<int>();
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(k201Created);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void SearchController::get_my(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "SELECT * FROM saved_searches WHERE user_id = 1 ORDER BY created_at DESC"
        );
        
        Json::Value searches(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value s;
            s["id"] = row["id"].as<int>();
            s["name"] = row["name"].as<std::string>();
            s["keywords"] = row["keywords"].as<std::string>();
            s["skills"] = row["skills"].as<std::string>();
            s["budget_min"] = row["budget_min"].isNull() ? 0 : row["budget_min"].as<double>();
            s["budget_max"] = row["budget_max"].isNull() ? 0 : row["budget_max"].as<double>();
            s["created_at"] = row["created_at"].as<std::string>();
            searches.append(s);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["searches"] = searches;
        
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

void SearchController::remove(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback,
                              int id) {
    try {
        auto dbClient = app().getDbClient("default");
        dbClient->execSqlSync("DELETE FROM saved_searches WHERE id = $1", id);
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Search deleted";
        
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
