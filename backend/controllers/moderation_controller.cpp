#include "moderation_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>

using namespace drogon;

// Очередь на модерацию
void ModerationController::get_queue(const HttpRequestPtr &req,
                                     std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "SELECT o.*, u.email as customer_email, c.name as category_name "
            "FROM orders o "
            "JOIN users u ON o.customer_id = u.id "
            "LEFT JOIN categories c ON o.category_id = c.id "
            "WHERE o.status IN ('on_moderation', 'draft') "
            "ORDER BY o.created_at ASC"
        );
        
        Json::Value orders(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value order;
            order["id"] = row["id"].as<int>();
            order["title"] = row["title"].as<std::string>();
            order["description"] = row["description"].as<std::string>();
            order["budget"] = row["budget"].isNull() ? 0 : row["budget"].as<double>();
            order["deadline"] = row["deadline"].isNull() ? "" : row["deadline"].as<std::string>();
            order["status"] = row["status"].as<std::string>();
            order["customer_email"] = row["customer_email"].as<std::string>();
            order["category"] = row["category_name"].isNull() ? "" : row["category_name"].as<std::string>();
            order["required_skills"] = row["required_skills"].as<std::string>();
            order["created_at"] = row["created_at"].as<std::string>();
            orders.append(order);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["orders"] = orders;
        response["total"] = (int)orders.size();
        
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

// Одобрить заказ
void ModerationController::approve(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback,
                                   int id) {
    auto json = req->getJsonObject();
    
    try {
        auto dbClient = app().getDbClient("default");
        
        // Проверяем статус заказа
        auto orderResult = dbClient->execSqlSync(
            "SELECT id, status, customer_id, title FROM orders WHERE id = $1", id
        );
        
        if (orderResult.size() == 0) {
            Json::Value error;
            error["error"] = "Order not found";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        auto orderRow = orderResult[0];
        std::string currentStatus = orderRow["status"].as<std::string>();
        
        if (currentStatus != "on_moderation" && currentStatus != "draft") {
            Json::Value error;
            error["error"] = "Order cannot be approved. Current status: " + currentStatus;
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        
        // Меняем статус на published
        dbClient->execSqlSync(
            "UPDATE orders SET status = 'published', updated_at = NOW() WHERE id = $1", id
        );
        
        // Записываем в историю
        dbClient->execSqlSync(
            "INSERT INTO order_status_history (order_id, old_status, new_status, changed_by, comment) "
            "VALUES ($1, $2, 'published', 1, $3)",
            id, currentStatus,
            (*json)["comment"].asString()
        );
        
        // Уведомление заказчику
        dbClient->execSqlSync(
            "INSERT INTO notifications (user_id, type, title, message) "
            "VALUES ($1, 'order_approved', 'Заказ опубликован', "
            "'Ваш заказ \"" + orderRow["title"].as<std::string>() + "\" прошёл модерацию и опубликован')",
            orderRow["customer_id"].as<int>()
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Order approved and published";
        
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

// Отклонить заказ
void ModerationController::reject(const HttpRequestPtr &req,
                                  std::function<void(const HttpResponsePtr &)> &&callback,
                                  int id) {
    auto json = req->getJsonObject();
    
    try {
        auto dbClient = app().getDbClient("default");
        
        auto orderResult = dbClient->execSqlSync(
            "SELECT id, status, customer_id, title FROM orders WHERE id = $1", id
        );
        
        if (orderResult.size() == 0) {
            Json::Value error;
            error["error"] = "Order not found";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        auto orderRow = orderResult[0];
        std::string reason = (*json)["reason"].asString();
        
        if (reason.empty()) {
            Json::Value error;
            error["error"] = "Rejection reason is required";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k400BadRequest);
            callback(resp);
            return;
        }
        
        dbClient->execSqlSync(
            "UPDATE orders SET status = 'rejected', updated_at = NOW() WHERE id = $1", id
        );
        
        // Записываем в историю
        dbClient->execSqlSync(
            "INSERT INTO order_status_history (order_id, old_status, new_status, changed_by, comment) "
            "VALUES ($1, $2, 'rejected', 1, $3)",
            id, orderRow["status"].as<std::string>(), reason
        );
        
        // Уведомление
        dbClient->execSqlSync(
            "INSERT INTO notifications (user_id, type, title, message) "
            "VALUES ($1, 'order_rejected', 'Заказ отклонён', $2)",
            orderRow["customer_id"].as<int>(),
            "Заказ \"" + orderRow["title"].as<std::string>() + "\" отклонён. Причина: " + reason
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Order rejected";
        
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

// Отправить на доработку
void ModerationController::request_revision(const HttpRequestPtr &req,
                                            std::function<void(const HttpResponsePtr &)> &&callback,
                                            int id) {
    auto json = req->getJsonObject();
    
    try {
        auto dbClient = app().getDbClient("default");
        
        auto orderResult = dbClient->execSqlSync(
            "SELECT id, status, customer_id, title FROM orders WHERE id = $1", id
        );
        
        if (orderResult.size() == 0) {
            Json::Value error;
            error["error"] = "Order not found";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }
        
        auto orderRow = orderResult[0];
        std::string comment = (*json)["comment"].asString();
        
        // Возвращаем в черновик с комментарием
        dbClient->execSqlSync(
            "UPDATE orders SET status = 'draft', updated_at = NOW() WHERE id = $1", id
        );
        
        // Записываем в историю
        dbClient->execSqlSync(
            "INSERT INTO order_status_history (order_id, old_status, new_status, changed_by, comment) "
            "VALUES ($1, $2, 'draft', 1, $3)",
            id, orderRow["status"].as<std::string>(), comment
        );
        
        // Уведомление
        dbClient->execSqlSync(
            "INSERT INTO notifications (user_id, type, title, message) "
            "VALUES ($1, 'order_revision', 'Заказ на доработке', $2)",
            orderRow["customer_id"].as<int>(),
            "Заказ \"" + orderRow["title"].as<std::string>() + "\" требует доработки: " + comment
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Order sent for revision";
        
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

// Список жалоб
void ModerationController::get_complaints(const HttpRequestPtr &req,
                                          std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "SELECT c.*, u.email as author_email "
            "FROM complaints c "
            "JOIN users u ON c.author_id = u.id "
            "WHERE c.status = 'pending' "
            "ORDER BY c.created_at ASC"
        );
        
        Json::Value complaints(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value complaint;
            complaint["id"] = row["id"].as<int>();
            complaint["author_email"] = row["author_email"].as<std::string>();
            complaint["target_type"] = row["target_type"].as<std::string>();
            complaint["target_id"] = row["target_id"].as<int>();
            complaint["reason"] = row["reason"].as<std::string>();
            complaint["created_at"] = row["created_at"].as<std::string>();
            complaints.append(complaint);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["complaints"] = complaints;
        response["total"] = (int)complaints.size();
        
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

// Разрешить жалобу
void ModerationController::resolve_complaint(const HttpRequestPtr &req,
                                             std::function<void(const HttpResponsePtr &)> &&callback,
                                             int id) {
    auto json = req->getJsonObject();
    
    try {
        auto dbClient = app().getDbClient("default");
        
        dbClient->execSqlSync(
            "UPDATE complaints SET status = 'reviewed', "
            "moderator_id = 1, resolution = $1, created_at = NOW() "
            "WHERE id = $2",
            (*json)["resolution"].asString(), id
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Complaint resolved";
        
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

// Проверка внешних заказов
void ModerationController::check_external_orders(const HttpRequestPtr &req,
                                                  std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "SELECT * FROM external_orders WHERE status = 'new' "
            "ORDER BY discovered_at DESC LIMIT 50"
        );
        
        Json::Value orders(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value order;
            order["id"] = row["id"].as<int>();
            order["title"] = row["title"].as<std::string>();
            order["description"] = row["description"].as<std::string>();
            order["source_name"] = row["source_name"].as<std::string>();
            order["source_url"] = row["source_url"].as<std::string>();
            order["budget"] = row["budget"].isNull() ? 0 : row["budget"].as<double>();
            order["discovered_at"] = row["discovered_at"].as<std::string>();
            orders.append(order);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["orders"] = orders;
        response["total"] = (int)orders.size();
        
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
