#include "role_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>

using namespace drogon;

void RoleController::request_role(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    try {
        auto db = app().getDbClient("default");
        int userId = (*json)["user_id"].asInt();
        std::string requestedRole = (*json)["role"].asString();
        
        auto existing = db->execSqlSync("SELECT id FROM role_requests WHERE user_id=$1 AND status='pending'", userId);
        if (existing.size() > 0) {
            Json::Value err; err["error"] = "У вас уже есть активная заявка";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k400BadRequest); callback(resp); return;
        }
        
        std::string company = json->isMember("company_name") ? (*json)["company_name"].asString() : "";
        std::string desc = json->isMember("description") ? (*json)["description"].asString() : "";
        
        db->execSqlSync(
            "INSERT INTO role_requests (user_id, requested_role, company_name, description) VALUES ($1, $2, $3, $4)",
            userId, requestedRole, company, desc
        );
        
        Json::Value resp; resp["status"] = "success"; resp["message"] = "Заявка отправлена на рассмотрение";
        auto httpResp = HttpResponse::newHttpJsonResponse(resp);
        httpResp->setStatusCode(k201Created); callback(httpResp);
    } catch (const std::exception& e) {
        Json::Value err; err["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void RoleController::get_requests(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto db = app().getDbClient("default");
        auto result = db->execSqlSync(
            "SELECT rr.*, u.email FROM role_requests rr JOIN users u ON rr.user_id = u.id ORDER BY rr.created_at DESC"
        );
        Json::Value requests(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value r;
            r["id"] = row["id"].as<int>();
            r["user_id"] = row["user_id"].as<int>();
            r["email"] = row["email"].as<std::string>();
            r["requested_role"] = row["requested_role"].as<std::string>();
            r["status"] = row["status"].as<std::string>();
            r["company_name"] = row["company_name"].isNull() ? "" : row["company_name"].as<std::string>();
            r["description"] = row["description"].isNull() ? "" : row["description"].as<std::string>();
            r["created_at"] = row["created_at"].as<std::string>();
            requests.append(r);
        }
        Json::Value resp; resp["status"] = "success"; resp["requests"] = requests;
        auto httpResp = HttpResponse::newHttpJsonResponse(resp); callback(httpResp);
    } catch (const std::exception& e) {
        Json::Value err; err["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void RoleController::approve(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto db = app().getDbClient("default");
        auto reqData = db->execSqlSync("SELECT * FROM role_requests WHERE id=$1", id);
        if (reqData.size() == 0) {
            Json::Value err; err["error"] = "Заявка не найдена";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k404NotFound); callback(resp); return;
        }
        int userId = reqData[0]["user_id"].as<int>();
        std::string newRole = reqData[0]["requested_role"].as<std::string>();
        std::string company = reqData[0]["company_name"].isNull() ? "" : reqData[0]["company_name"].as<std::string>();
        std::string desc = reqData[0]["description"].isNull() ? "" : reqData[0]["description"].as<std::string>();
        
        db->execSqlSync("UPDATE users SET role=$1 WHERE id=$2", newRole, userId);
        if (newRole == "customer" && !company.empty()) { auto cp = db->execSqlSync("SELECT id FROM customer_profiles WHERE user_id=$1", userId); if (cp.size() > 0) { db->execSqlSync("UPDATE customer_profiles SET company_name=$1, description=$2 WHERE user_id=$3", company, desc, userId); } else { db->execSqlSync("INSERT INTO customer_profiles (user_id, company_name, description) VALUES ($1, $2, $3)", userId, company, desc); } }
        db->execSqlSync("UPDATE role_requests SET status='approved' WHERE id=$1", id);
        
        if (newRole == "customer") {
            db->execSqlSync("INSERT INTO customer_profiles (user_id, company_name, description) VALUES ($1, $2, $3) ", userId, company, desc);
        db->execSqlSync("DELETE FROM role_requests WHERE user_id=$1 AND status='pending'", userId);
        }
        
        Json::Value resp; resp["status"] = "success"; resp["message"] = "Заявка одобрена";
        auto httpResp = HttpResponse::newHttpJsonResponse(resp); callback(httpResp);
    } catch (const std::exception& e) {
        Json::Value err; err["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void RoleController::reject(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto db = app().getDbClient("default");
        db->execSqlSync("UPDATE role_requests SET status='rejected' WHERE id=$1", id);
        Json::Value resp; resp["status"] = "success";
        auto httpResp = HttpResponse::newHttpJsonResponse(resp); callback(httpResp);
    } catch (const std::exception& e) {
        Json::Value err; err["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}
