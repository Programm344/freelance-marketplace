#include "review_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include "../utils/request_helper.h"

using namespace drogon;

void ReviewController::create(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    try {
        auto dbClient = app().getDbClient("default");
        int authorId = getUserIdFromRequest(req);
        int targetId = (*json)["target_id"].asInt();
        int orderId = (*json)["order_id"].asInt();
        int rating = (*json)["rating"].asInt();
        std::string comment = (*json)["comment"].asString();
        
        // Проверка заказа
        auto orderCheck = dbClient->execSqlSync("SELECT status FROM orders WHERE id = $1", orderId);
        if (orderCheck.size() == 0 || orderCheck[0]["status"].as<std::string>() != "completed") {
            Json::Value error; error["error"] = "Order must be completed";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest); callback(resp); return;
        }
        
        // Проверка дубликата (author + target + order)
        auto existing = dbClient->execSqlSync(
            "SELECT id FROM reviews WHERE author_id=$1 AND order_id=$2 AND target_id=$3",
            authorId, orderId, targetId
        );
        if (existing.size() > 0) {
            Json::Value error; error["error"] = "Вы уже оставили отзыв этому пользователю";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest); callback(resp); return;
        }
        
        auto result = dbClient->execSqlSync(
            "INSERT INTO reviews (author_id, target_id, order_id, rating, comment) VALUES ($1,$2,$3,$4,$5) RETURNING id",
            authorId, targetId, orderId, rating, comment
        );
        
        // Обновляем рейтинг
        auto avg = dbClient->execSqlSync("SELECT AVG(rating)::numeric(3,2) as avg FROM reviews WHERE target_id=$1", targetId);
        double avgRating = avg[0]["avg"].isNull() ? rating : avg[0]["avg"].as<double>();
        dbClient->execSqlSync("UPDATE freelancer_profiles SET rating=$1 WHERE user_id=$2", avgRating, targetId);
        dbClient->execSqlSync("UPDATE customer_profiles SET rating=$1 WHERE user_id=$2", avgRating, targetId);
        
        dbClient->execSqlSync("INSERT INTO notifications (user_id, type, title, message) VALUES ($1, 'new_review', 'Новый отзыв', 'Вам оставили отзыв')", targetId);
        
        Json::Value resp; resp["status"] = "success"; resp["review_id"] = result[0]["id"].as<int>(); resp["new_rating"] = avgRating;
        auto httpResp = HttpResponse::newHttpJsonResponse(resp);
        httpResp->setStatusCode(k201Created); callback(httpResp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void ReviewController::get_for_user(const HttpRequestPtr &req,
                                    std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT rv.*, u.email as author_email, o.title as order_title FROM reviews rv "
            "JOIN users u ON rv.author_id = u.id LEFT JOIN orders o ON rv.order_id = o.id "
            "WHERE rv.target_id = $1 ORDER BY rv.created_at DESC LIMIT 50", id
        );
        Json::Value reviews(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value r;
            r["id"] = row["id"].as<int>();
            r["author"] = row["author_email"].as<std::string>();
            r["order_title"] = row["order_title"].isNull() ? "" : row["order_title"].as<std::string>();
            r["rating"] = row["rating"].as<int>();
            r["comment"] = row["comment"].as<std::string>();
            r["created_at"] = row["created_at"].as<std::string>();
            reviews.append(r);
        }
        auto avg = dbClient->execSqlSync("SELECT AVG(rating)::numeric(3,2) as avg, COUNT(*) as total FROM reviews WHERE target_id=$1", id);
        Json::Value resp; resp["status"] = "success"; resp["reviews"] = reviews;
        resp["avg_rating"] = avg[0]["avg"].isNull() ? 0.0 : avg[0]["avg"].as<double>();
        resp["total"] = avg[0]["total"].as<int>();
        auto httpResp = HttpResponse::newHttpJsonResponse(resp); callback(httpResp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void ReviewController::get_for_order(const HttpRequestPtr &req,
                                      std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto db = app().getDbClient("default");
        auto result = db->execSqlSync(
            "SELECT rv.*, u.email as author_email FROM reviews rv "
            "JOIN users u ON rv.author_id = u.id WHERE rv.order_id = $1 ORDER BY rv.created_at DESC", id
        );
        Json::Value reviews(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value r;
            r["id"] = row["id"].as<int>();
            r["author"] = row["author_email"].as<std::string>();
            r["rating"] = row["rating"].as<int>();
            r["comment"] = row["comment"].as<std::string>();
            r["created_at"] = row["created_at"].as<std::string>();
            reviews.append(r);
        }
        Json::Value resp; resp["status"] = "success"; resp["reviews"] = reviews;
        auto httpResp = HttpResponse::newHttpJsonResponse(resp); callback(httpResp);
    } catch (const std::exception& e) {
        Json::Value err; err["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}
