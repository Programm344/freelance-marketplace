#include "admin_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include <sstream>

using namespace drogon;

void AdminController::get_users(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT u.*, fp.display_name, fp.rating as freelancer_rating, "
            "cp.company_name, cp.rating as customer_rating "
            "FROM users u "
            "LEFT JOIN freelancer_profiles fp ON u.id = fp.user_id "
            "LEFT JOIN customer_profiles cp ON u.id = cp.user_id "
            "ORDER BY u.created_at DESC"
        );
        
        Json::Value users(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value user;
            user["id"] = row["id"].as<int>();
            user["email"] = row["email"].as<std::string>();
            user["role"] = row["role"].as<std::string>();
            user["is_blocked"] = row["is_blocked"].as<bool>();
            user["display_name"] = row["display_name"].isNull() ? "" : row["display_name"].as<std::string>();
            user["company_name"] = row["company_name"].isNull() ? "" : row["company_name"].as<std::string>();
            user["created_at"] = row["created_at"].as<std::string>();
            users.append(user);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["users"] = users;
        response["total"] = (int)users.size();
        
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

void AdminController::update_user(const HttpRequestPtr &req,
                                  std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    auto json = req->getJsonObject();
    try {
        auto dbClient = app().getDbClient("default");
        dbClient->execSqlSync("UPDATE users SET role=$1 WHERE id=$2",
            (*json)["role"].asString(), id);
        
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

void AdminController::block_user(const HttpRequestPtr &req,
                                 std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto dbClient = app().getDbClient("default");
        dbClient->execSqlSync("UPDATE users SET is_blocked=true WHERE id=$1", id);
        Json::Value response;
        response["status"] = "success";
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void AdminController::unblock_user(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto dbClient = app().getDbClient("default");
        dbClient->execSqlSync("UPDATE users SET is_blocked=false WHERE id=$1", id);
        Json::Value response;
        response["status"] = "success";
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void AdminController::get_stats(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        Json::Value stats;
        
        auto r1 = dbClient->execSqlSync("SELECT COUNT(*) as c FROM orders WHERE status='published'");
        stats["published_orders"] = r1[0]["c"].as<int>();
        
        auto r2 = dbClient->execSqlSync("SELECT COUNT(*) as c FROM orders WHERE status='on_moderation'");
        stats["moderation_orders"] = r2[0]["c"].as<int>();
        
        auto r3 = dbClient->execSqlSync("SELECT COUNT(*) as c FROM orders WHERE status='in_progress'");
        stats["in_progress_orders"] = r3[0]["c"].as<int>();
        
        auto r4 = dbClient->execSqlSync("SELECT COUNT(*) as c FROM orders WHERE status='completed'");
        stats["completed_orders"] = r4[0]["c"].as<int>();
        
        auto r5 = dbClient->execSqlSync("SELECT COUNT(*) as c FROM responses");
        stats["total_responses"] = r5[0]["c"].as<int>();
        
        auto r6 = dbClient->execSqlSync("SELECT COUNT(*) as c FROM external_orders WHERE status='new'");
        stats["new_external_orders"] = r6[0]["c"].as<int>();
        
        auto r7 = dbClient->execSqlSync("SELECT COUNT(*) as c FROM crawl_logs WHERE status='error'");
        stats["crawl_errors"] = r7[0]["c"].as<int>();
        
        auto r8 = dbClient->execSqlSync("SELECT COUNT(*) as c FROM crawler_sources WHERE status='active'");
        stats["active_sources"] = r8[0]["c"].as<int>();
        
        auto r9 = dbClient->execSqlSync("SELECT COUNT(*) as c FROM users");
        stats["total_users"] = r9[0]["c"].as<int>();
        
        Json::Value response;
        response["status"] = "success";
        response["stats"] = stats;
        
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

void AdminController::export_csv(const HttpRequestPtr &req,
                                 std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT o.id, o.title, o.status, o.budget, o.created_at, u.email "
            "FROM orders o JOIN users u ON o.customer_id=u.id ORDER BY o.created_at DESC"
        );
        
        std::stringstream csv;
        csv << "ID,Title,Status,Budget,Created At,Customer\n";
        for (const auto& row : result) {
            csv << row["id"].as<int>() << ","
                << "\"" << row["title"].as<std::string>() << "\","
                << row["status"].as<std::string>() << ","
                << row["budget"].as<double>() << ","
                << row["created_at"].as<std::string>() << ","
                << row["email"].as<std::string>() << "\n";
        }
        
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody(csv.str());
        resp->setContentTypeString("text/csv; charset=utf-8");
        resp->addHeader("Content-Disposition", "attachment; filename=report.csv");
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Export failed: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}
