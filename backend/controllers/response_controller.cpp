#include "../utils/request_helper.h"
#include "response_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>

using namespace drogon;

// Отправить отклик
void ResponseController::create(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    
    if (!json) {
        Json::Value error;
        error["error"] = "Invalid JSON";
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    
    try {
        auto dbClient = app().getDbClient("default");
        
        int orderId = (*json)["order_id"].asInt();
        int freelancerId = (*json)["freelancer_id"].asInt(); // TODO: брать из токена
        std::string message = (*json)["message"].asString();
        double proposedBudget = (*json)["proposed_budget"].asDouble();
        std::string proposedDeadline = (*json)["proposed_deadline"].asString();
        
        // Проверяем что заказ существует и опубликован
        auto orderCheck = dbClient->execSqlSync(
            "SELECT id, status, customer_id FROM orders WHERE id = $1", orderId
        );
        
        if (orderCheck.size() == 0) {
            Json::Value error;
            error["error"] = "Order not found";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        auto orderRow = orderCheck[0];
        std::string orderStatus = orderRow["status"].as<std::string>();
        
        if (orderStatus != "published") {
            Json::Value error;
            error["error"] = "Order is not available for responses";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        
        // Проверяем что фрилансер не откликается на свой же заказ
        if (orderRow["customer_id"].as<int>() == freelancerId) {
            Json::Value error;
            error["error"] = "You cannot respond to your own order";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        
        // Проверяем что нет уже активного отклика от этого фрилансера
        auto existingResponse = dbClient->execSqlSync(
            "SELECT id FROM responses WHERE order_id = $1 AND freelancer_id = $2 "
            "AND status NOT IN ('withdrawn', 'rejected')",
            orderId, freelancerId
        );
        
        if (existingResponse.size() > 0) {
            Json::Value error;
            error["error"] = "You already have an active response for this order";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        
        // Создаём отклик
        auto result = dbClient->execSqlSync(
            "INSERT INTO responses (order_id, freelancer_id, message, proposed_budget, "
            "proposed_deadline, status) "
            "VALUES ($1, $2, $3, $4, $5, 'sent') RETURNING id, created_at",
            orderId, freelancerId, message, proposedBudget, proposedDeadline
        );
        
        // Создаём уведомление заказчику
        dbClient->execSqlSync(
            "INSERT INTO notifications (user_id, type, title, message, data) "
            "VALUES ($1, 'new_response', 'Новый отклик на заказ', "
            "$2, $3)",
            orderRow["customer_id"].as<int>(),
            "Фрилансер откликнулся на ваш заказ",
            Json::Value().toStyledString()
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Response sent successfully";
        response["response_id"] = result[0]["id"].as<int>();
        response["created_at"] = result[0]["created_at"].as<std::string>();
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(k201Created);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed to create response: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

// Получить отклики на заказ
void ResponseController::get_by_order(const HttpRequestPtr &req,
                                      std::function<void(const HttpResponsePtr &)> &&callback,
                                      int order_id) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "SELECT r.*, u.email as freelancer_email, fp.display_name, fp.rating "
            "FROM responses r "
            "JOIN users u ON r.freelancer_id = u.id "
            "LEFT JOIN freelancer_profiles fp ON u.id = fp.user_id "
            "WHERE r.order_id = $1 "
            "ORDER BY r.created_at DESC",
            order_id
        );
        
        Json::Value responses(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value resp;
            resp["id"] = row["id"].as<int>();
            resp["order_id"] = row["order_id"].as<int>();
            resp["freelancer_id"] = row["freelancer_id"].as<int>();
            resp["freelancer_email"] = row["freelancer_email"].as<std::string>();
            resp["display_name"] = row["display_name"].isNull() ? "" : row["display_name"].as<std::string>();
            resp["rating"] = row["rating"].isNull() ? 0.0 : row["rating"].as<double>();
            resp["message"] = row["message"].as<std::string>();
            resp["proposed_budget"] = row["proposed_budget"].isNull() ? 0 : row["proposed_budget"].as<double>();
            resp["proposed_deadline"] = row["proposed_deadline"].isNull() ? "" : row["proposed_deadline"].as<std::string>();
            resp["status"] = row["status"].as<std::string>();
            resp["created_at"] = row["created_at"].as<std::string>();
            responses.append(resp);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["responses"] = responses;
        response["total"] = (int)responses.size();
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed to fetch responses: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

// Получить мои отклики (фрилансер)
void ResponseController::get_my_responses(const HttpRequestPtr &req,
                                          std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        int freelancerId = 1; // TODO: брать из токена
        
        auto result = dbClient->execSqlSync(
            "SELECT r.*, o.title as order_title, o.budget as order_budget, "
            "o.status as order_status, u.email as customer_email "
            "FROM responses r "
            "JOIN orders o ON r.order_id = o.id "
            "JOIN users u ON o.customer_id = u.id "
            "WHERE r.freelancer_id = $1 "
            "ORDER BY r.created_at DESC",
            freelancerId
        );
        
        Json::Value responses(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value resp;
            resp["id"] = row["id"].as<int>();
            resp["order_id"] = row["order_id"].as<int>();
            resp["order_title"] = row["order_title"].as<std::string>();
            resp["order_budget"] = row["order_budget"].isNull() ? 0 : row["order_budget"].as<double>();
            resp["order_status"] = row["order_status"].as<std::string>();
            resp["customer_email"] = row["customer_email"].as<std::string>();
            resp["message"] = row["message"].as<std::string>();
            resp["proposed_budget"] = row["proposed_budget"].isNull() ? 0 : row["proposed_budget"].as<double>();
            resp["proposed_deadline"] = row["proposed_deadline"].isNull() ? "" : row["proposed_deadline"].as<std::string>();
            resp["status"] = row["status"].as<std::string>();
            resp["created_at"] = row["created_at"].as<std::string>();
            responses.append(resp);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["responses"] = responses;
        response["total"] = (int)responses.size();
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed to fetch responses: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

// Принять отклик
void ResponseController::accept(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback,
                                int id) {
    try {
        auto dbClient = app().getDbClient("default");
        
        // Получаем отклик
        auto responseCheck = dbClient->execSqlSync(
            "SELECT r.*, o.customer_id, o.status as order_status "
            "FROM responses r "
            "JOIN orders o ON r.order_id = o.id "
            "WHERE r.id = $1", id
        );
        
        if (responseCheck.size() == 0) {
            Json::Value error;
            error["error"] = "Response not found";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        auto row = responseCheck[0];
        
        // Проверяем что заказ ещё опубликован
        if (row["order_status"].as<std::string>() != "published") {
            Json::Value error;
            error["error"] = "Order is not available";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        
        // Принимаем отклик и переводим заказ в работу
        dbClient->execSqlSync(
            "UPDATE responses SET status = 'accepted', updated_at = NOW() WHERE id = $1", id
        );
        
        dbClient->execSqlSync(
            "UPDATE orders SET status = 'in_progress', updated_at = NOW() WHERE id = $1",
            row["order_id"].as<int>()
        );
        
        // Отклоняем остальные отклики
        dbClient->execSqlSync(
            "UPDATE responses SET status = 'rejected', updated_at = NOW() "
            "WHERE order_id = $1 AND id != $2 AND status = 'sent'",
            row["order_id"].as<int>(), id
        );
        
        // Уведомление фрилансеру
        dbClient->execSqlSync(
            "INSERT INTO notifications (user_id, type, title, message) "
            "VALUES ($1, 'response_accepted', 'Отклик принят', 'Ваш отклик на заказ принят!')",
            row["freelancer_id"].as<int>()
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Response accepted, order moved to in_progress";
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed to accept response: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

// Отклонить отклик
void ResponseController::reject(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback,
                                int id) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "UPDATE responses SET status = 'rejected', updated_at = NOW() "
            "WHERE id = $1 AND status = 'sent' RETURNING freelancer_id, order_id",
            id
        );
        
        if (result.size() == 0) {
            Json::Value error;
            error["error"] = "Response not found or cannot be rejected";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        // Уведомление фрилансеру
        dbClient->execSqlSync(
            "INSERT INTO notifications (user_id, type, title, message) "
            "VALUES ($1, 'response_rejected', 'Отклик отклонён', 'Ваш отклик был отклонён заказчиком')",
            result[0]["freelancer_id"].as<int>()
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Response rejected";
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed to reject response: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

// Отозвать отклик
void ResponseController::withdraw(const HttpRequestPtr &req,
                                  std::function<void(const HttpResponsePtr &)> &&callback,
                                  int id) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "UPDATE responses SET status = 'withdrawn', updated_at = NOW() "
            "WHERE id = $1 AND status = 'sent' RETURNING id",
            id
        );
        
        if (result.size() == 0) {
            Json::Value error;
            error["error"] = "Response not found or cannot be withdrawn";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Response withdrawn";
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed to withdraw response: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}
