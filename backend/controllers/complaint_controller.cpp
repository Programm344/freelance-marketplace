#include "../utils/request_helper.h"
#include "complaint_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>

using namespace drogon;

void ComplaintController::create(const HttpRequestPtr &req,
                                 std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "INSERT INTO complaints (author_id, target_type, target_id, reason) "
            "VALUES ($1, $2, $3, $4) RETURNING id",
            1, // TODO: из токена
            (*json)["target_type"].asString(),
            (*json)["target_id"].asInt(),
            (*json)["reason"].asString()
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Complaint sent";
        response["complaint_id"] = result[0]["id"].as<int>();
        
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

void ComplaintController::get_my(const HttpRequestPtr &req,
                                 std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "SELECT * FROM complaints WHERE author_id = 1 ORDER BY created_at DESC"
        );
        
        Json::Value complaints(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value c;
            c["id"] = row["id"].as<int>();
            c["target_type"] = row["target_type"].as<std::string>();
            c["target_id"] = row["target_id"].as<int>();
            c["reason"] = row["reason"].as<std::string>();
            c["status"] = row["status"].as<std::string>();
            c["resolution"] = row["resolution"].isNull() ? "" : row["resolution"].as<std::string>();
            c["created_at"] = row["created_at"].as<std::string>();
            complaints.append(c);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["complaints"] = complaints;
        
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
