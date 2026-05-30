#include "../utils/request_helper.h"
#include "review_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>

using namespace drogon;

void ReviewController::create(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    
    try {
        auto dbClient = app().getDbClient("default");
        
        int authorId = 1; // TODO: из токена
        int targetId = (*json)["target_id"].asInt();
        int orderId = (*json)["order_id"].asInt();
        int rating = (*json)["rating"].asInt();
        std::string comment = (*json)["comment"].asString();
        
        // Проверяем что заказ завершён
        auto orderCheck = dbClient->execSqlSync(
            "SELECT status FROM orders WHERE id = $1", orderId
        );
        
        if (orderCheck.size() == 0 || orderCheck[0]["status"].as<std::string>() != "completed") {
            Json::Value error;
            error["error"] = "Order must be completed to leave a review";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        
        // Проверяем что отзыва ещё нет от этого автора на этот заказ
        auto existingReview = dbClient->execSqlSync(
            "SELECT id FROM reviews WHERE author_id=$1 AND order_id=$2", authorId, orderId
        );
        if (existingReview.size() > 0) {
            Json::Value error;
            error["error"] = "Вы уже оставили отзыв на этот заказ";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        
        // Создаём отзыв
        auto result = dbClient->execSqlSync(
            "INSERT INTO reviews (author_id, target_id, order_id, rating, comment) "
            "VALUES ($1, $2, $3, $4, $5) RETURNING id",
            authorId, targetId, orderId, rating, comment
        );
        
        // Обновляем рейтинг пользователя
        auto avgResult = dbClient->execSqlSync(
            "SELECT AVG(rating)::numeric(3,2) as avg_rating FROM reviews WHERE target_id = $1",
            targetId
        );
        
        double avgRating = avgResult[0]["avg_rating"].isNull() ? rating : avgResult[0]["avg_rating"].as<double>();
        
        // Обновляем в соответствующем профиле
        dbClient->execSqlSync(
            "UPDATE freelancer_profiles SET rating = $1, updated_at = NOW() WHERE user_id = $2",
            avgRating, targetId
        );
        dbClient->execSqlSync(
            "UPDATE customer_profiles SET rating = $1, updated_at = NOW() WHERE user_id = $2",
            avgRating, targetId
        );
        
        // Уведомление
        dbClient->execSqlSync(
            "INSERT INTO notifications (user_id, type, title, message) "
            "VALUES ($1, 'new_review', 'Новый отзыв', 'Вам оставили отзыв с оценкой ' || $2)",
            targetId, std::to_string(rating)
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Review created";
        response["review_id"] = result[0]["id"].as<int>();
        response["new_rating"] = avgRating;
        
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

void ReviewController::get_for_user(const HttpRequestPtr &req,
                                    std::function<void(const HttpResponsePtr &)> &&callback,
                                    int id) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "SELECT rv.*, u.email as author_email, o.title as order_title "
            "FROM reviews rv "
            "JOIN users u ON rv.author_id = u.id "
            "LEFT JOIN orders o ON rv.order_id = o.id "
            "WHERE rv.target_id = $1 "
            "ORDER BY rv.created_at DESC LIMIT 50",
            id
        );
        
        Json::Value reviews(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value review;
            review["id"] = row["id"].as<int>();
            review["author"] = row["author_email"].as<std::string>();
            review["order_title"] = row["order_title"].isNull() ? "" : row["order_title"].as<std::string>();
            review["rating"] = row["rating"].as<int>();
            review["comment"] = row["comment"].as<std::string>();
            review["created_at"] = row["created_at"].as<std::string>();
            reviews.append(review);
        }
        
        // Средний рейтинг
        auto avgResult = dbClient->execSqlSync(
            "SELECT AVG(rating)::numeric(3,2) as avg_rating, COUNT(*) as total "
            "FROM reviews WHERE target_id = $1", id
        );
        
        Json::Value response;
        response["status"] = "success";
        response["reviews"] = reviews;
        response["avg_rating"] = avgResult[0]["avg_rating"].isNull() ? 0.0 : avgResult[0]["avg_rating"].as<double>();
        response["total"] = avgResult[0]["total"].as<int>();
        
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
