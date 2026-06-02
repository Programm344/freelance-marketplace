#include "../utils/request_helper.h"
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
        std::string company = json->isMember("company_name") ? (*json)["company_name"].asString() : "";
        std::string desc = json->isMember("description") ? (*json)["description"].asString() : "";
        
// Для админа и фрилансера — меняем роль сразу
        if (requestedRole == "admin") {
            // Только админ может назначать админов
            auto userCheck = db->execSqlSync("SELECT role FROM users WHERE id=$1", userId);
            if (userCheck.size() == 0 || userCheck[0]["role"].as<std::string>() != "admin") {
                Json::Value err; err["error"] = "Только админ может назначить админа";
                auto resp = HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(k403Forbidden); callback(resp); return;
            }
            db->execSqlSync("UPDATE users SET role='admin' WHERE id=$2", userId);
            Json::Value resp; resp["status"] = "success"; resp["message"] = "Роль изменена на admin";
            auto httpResp = HttpResponse::newHttpJsonResponse(resp); callback(httpResp); return;
        }
        
        if (requestedRole == "freelancer") {
            db->execSqlSync("UPDATE users SET role=$1 WHERE id=$2", requestedRole, userId);
            Json::Value resp; resp["status"] = "success";
            resp["message"] = "Роль изменена на " + requestedRole;
            auto httpResp = HttpResponse::newHttpJsonResponse(resp);
            callback(httpResp); return;
        }
        
        // Если уже был заказчиком и профиль заполнен — меняем сразу
        if (requestedRole == "customer") {
            auto profile = db->execSqlSync("SELECT company_name FROM customer_profiles WHERE user_id = $1", userId);
            if (profile.size() > 0 && !profile[0]["company_name"].isNull() && !profile[0]["company_name"].as<std::string>().empty()) {
                db->execSqlSync("UPDATE users SET role='customer' WHERE id=$1", userId);
                Json::Value resp; resp["status"] = "success";
                resp["message"] = "Роль изменена на customer (профиль уже заполнен)";
                auto httpResp = HttpResponse::newHttpJsonResponse(resp);
                callback(httpResp); return;
            }
        }
        
        // Для заказчика — заявка с формой
        // Проверяем что нет активной заявки
        auto existing = db->execSqlSync(
            "SELECT id FROM role_requests WHERE user_id=$1 AND status='pending'", userId
        );
        if (existing.size() > 0) {
            Json::Value err; err["error"] = "У вас уже есть активная заявка";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k400BadRequest); callback(resp); return;
        }
        
        db->execSqlSync(
            "INSERT INTO role_requests (user_id, requested_role) VALUES ($1, $2)",
            userId, requestedRole
        );
        
        // Уведомление модераторам
        db->execSqlSync(
            "INSERT INTO notifications (user_id, type, title, message) "
            "SELECT id, 'role_request', 'Заявка на смену роли', "
            "'Пользователь хочет стать ' || $1 "
            "FROM users WHERE role IN ('moderator', 'admin')",
            requestedRole
        );
        
        Json::Value resp; resp["status"] = "success";
        resp["message"] = "Заявка отправлена на рассмотрение";
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
            "SELECT rr.*, u.email FROM role_requests rr "
            "JOIN users u ON rr.user_id = u.id "
            "WHERE rr.status = 'pending' ORDER BY rr.created_at DESC"
        );
        
        Json::Value requests(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value r;
            r["id"] = row["id"].as<int>();
            r["user_id"] = row["user_id"].as<int>();
            r["email"] = row["email"].as<std::string>();
            r["requested_role"] = row["requested_role"].as<std::string>();
            r["created_at"] = row["created_at"].as<std::string>();
            requests.append(r);
        }
        
        Json::Value resp; resp["status"] = "success";
        resp["requests"] = requests;
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
        
        auto req = db->execSqlSync("SELECT * FROM role_requests WHERE id=$1", id);
        if (req.size() == 0) {
            Json::Value err; err["error"] = "Заявка не найдена";
            auto resp = HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(k404NotFound); callback(resp); return;
        }
        
        int userId = req[0]["user_id"].as<int>();
        std::string newRole = req[0]["requested_role"].as<std::string>();
        
        // Меняем роль
        db->execSqlSync("UPDATE users SET role=$1 WHERE id=$2", newRole, userId);
        db->execSqlSync("UPDATE role_requests SET status='approved' WHERE id=$1", id);
        
        // Создаём профиль если заказчик
        if (newRole == "customer") {
            db->execSqlSync(
                "INSERT INTO customer_profiles (user_id) VALUES ($1) ON CONFLICT DO NOTHING", userId
            );
        }
        
        Json::Value resp; resp["status"] = "success";
        resp["message"] = "Заявка одобрена";
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
