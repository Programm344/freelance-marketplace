#include "../utils/request_helper.h"
#include "order_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include "../utils/helpers.h"

using namespace drogon;

void OrderController::get_my_orders(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback) {
    auto params = req->getParameters();
    int userId = params.find("user_id") != params.end() ? std::stoi(params["user_id"]) : 1;
    
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT o.id, o.title, o.description, o.budget, o.deadline, o.status, "
            "o.created_at, u.email as customer_email, c.name as category_name "
            "FROM orders o JOIN users u ON o.customer_id = u.id "
            "LEFT JOIN categories c ON o.category_id = c.id "
            "WHERE o.customer_id = $1 "
            "ORDER BY o.created_at DESC LIMIT 50", userId
        );
        
        Json::Value orders(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value order;
            order["id"] = row["id"].as<int>();
            order["title"] = row["title"].as<std::string>();
            order["description"] = row["description"].isNull() ? "" : row["description"].as<std::string>();
            order["budget"] = row["budget"].isNull() ? 0.0 : row["budget"].as<double>();
            order["status"] = row["status"].as<std::string>();
            order["customer_email"] = row["customer_email"].as<std::string>();
            order["category"] = row["category_name"].isNull() ? "" : row["category_name"].as<std::string>();
            order["created_at"] = row["created_at"].as<std::string>();
            orders.append(order);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["orders"] = orders;
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void OrderController::get_all(const HttpRequestPtr &req,
                              std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT o.id, o.title, o.description, o.budget, o.deadline, o.status, "
            "o.created_at, o.required_skills, o.payment_format, o.source, "
            "u.email as customer_email, c.name as category_name "
            "FROM orders o JOIN users u ON o.customer_id = u.id "
            "LEFT JOIN categories c ON o.category_id = c.id "
            "ORDER BY o.created_at DESC LIMIT 50"
        );
        
        Json::Value orders(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value order;
            order["id"] = row["id"].as<int>();
            order["title"] = row["title"].as<std::string>();
            order["description"] = row["description"].isNull() ? "" : row["description"].as<std::string>();
            order["budget"] = row["budget"].isNull() ? 0.0 : row["budget"].as<double>();
            order["deadline"] = row["deadline"].isNull() ? "" : row["deadline"].as<std::string>();
            order["status"] = row["status"].as<std::string>();
            order["customer_email"] = row["customer_email"].as<std::string>();
            order["category"] = row["category_name"].isNull() ? "" : row["category_name"].as<std::string>();
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
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void OrderController::get_by_id(const HttpRequestPtr &req,
                                std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT o.*, u.email as customer_email, c.name as category_name "
            "FROM orders o JOIN users u ON o.customer_id = u.id "
            "LEFT JOIN categories c ON o.category_id = c.id WHERE o.id = $1", id
        );
        
        if (result.size() == 0) {
            Json::Value error; error["error"] = "Order not found";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound); callback(resp); return;
        }
        
        auto row = result[0];
        Json::Value order;
        order["id"] = row["id"].as<int>();
        order["title"] = row["title"].as<std::string>();
        order["description"] = row["description"].as<std::string>();
        order["budget"] = row["budget"].isNull() ? 0.0 : row["budget"].as<double>();
        order["deadline"] = row["deadline"].isNull() ? "" : row["deadline"].as<std::string>();
        order["status"] = row["status"].as<std::string>();
        order["required_skills"] = row["required_skills"].isNull() ? "" : row["required_skills"].as<std::string>();
        order["customer_email"] = row["customer_email"].as<std::string>();
        order["customer_id"] = row["customer_id"].as<int>();
        order["category"] = row["category_name"].isNull() ? "" : row["category_name"].as<std::string>();
        order["created_at"] = row["created_at"].as<std::string>();
        
        Json::Value response;
        response["status"] = "success";
        response["order"] = order;
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void OrderController::create(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    try {
        auto dbClient = app().getDbClient("default");
        std::string skills = toPgArray((*json)["required_skills"].asString());
        int customerId = (*json)["customer_id"].asInt();
        if (customerId == 0) customerId = 1;
        
        auto result = dbClient->execSqlSync(
            "INSERT INTO orders (customer_id, title, description, category_id, "
            "required_skills, budget, deadline, payment_format, status) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, 'on_moderation') RETURNING id",
            customerId, (*json)["title"].asString(), (*json)["description"].asString(),
            (*json)["category_id"].asInt(), skills, (*json)["budget"].asDouble(),
            (*json)["deadline"].asString(), (*json)["payment_format"].asString()
        );
        
        Json::Value response;
        response["status"] = "success";
        response["order_id"] = result[0]["id"].as<int>();
        auto resp = HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(k201Created); callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void OrderController::update(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    auto json = req->getJsonObject();
    try {
        auto dbClient = app().getDbClient("default");
        std::string newStatus = (*json)["status"].asString();
        
        if (!newStatus.empty()) {
            dbClient->execSqlSync(
                "UPDATE orders SET status=$1::order_status, updated_at=NOW() WHERE id=$2",
                newStatus, id
            );
        }
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Order updated";
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}


void OrderController::submit_to_moderation(const HttpRequestPtr &req,
                                           std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto dbClient = app().getDbClient("default");
        dbClient->execSqlSync(
            "UPDATE orders SET status='on_moderation'::order_status, updated_at=NOW() WHERE id=$1", id
        );
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Order sent to moderation";
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void OrderController::delete_order(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto dbClient = app().getDbClient("default");
        dbClient->execSqlSync("DELETE FROM orders WHERE id=$1", id);
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Order deleted";
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void OrderController::search(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback) {
    auto params = req->getParameters();
    try {
        auto dbClient = app().getDbClient("default");
        
        std::string keyword = params.find("keyword") != params.end() ? params["keyword"] : "";
        std::string budgetMin = params.find("budget_min") != params.end() ? params["budget_min"] : "";
        std::string budgetMax = params.find("budget_max") != params.end() ? params["budget_max"] : "";
        
        // Безопасный запрос с параметрами
        std::string query = 
            "SELECT o.id, o.title, o.budget, o.deadline, o.status, o.created_at, "
            "u.email as customer_email, c.name as category_name "
            "FROM orders o JOIN users u ON o.customer_id = u.id "
            "LEFT JOIN categories c ON o.category_id = c.id "
            "WHERE o.status NOT IN ('draft', 'on_moderation')";
        
        if (!keyword.empty())
            query += " AND (o.title ILIKE $1 OR o.description ILIKE $1 OR o.required_skills::text ILIKE $1)";
        if (!budgetMin.empty())
            query += " AND o.budget >= " + budgetMin;  // числа безопасны после проверки
        if (!budgetMax.empty())
            query += " AND o.budget <= " + budgetMax;
        
        std::string status = params.find("status") != params.end() ? params["status"] : "";
        if (!status.empty())
            query += " AND o.status = '" + status + "'";
        
        query += " ORDER BY o.created_at DESC LIMIT 50";
        
        auto result = keyword.empty() ? 
            dbClient->execSqlSync(query) :
            dbClient->execSqlSync(query, "%" + keyword + "%");
        
        Json::Value orders(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value order;
            order["id"] = row["id"].as<int>();
            order["title"] = row["title"].as<std::string>();
            order["budget"] = row["budget"].isNull() ? 0.0 : row["budget"].as<double>();
            order["status"] = row["status"].as<std::string>();
            order["customer_email"] = row["customer_email"].as<std::string>();
            order["category"] = row["category_name"].isNull() ? "" : row["category_name"].as<std::string>();
            order["created_at"] = row["created_at"].as<std::string>();
            orders.append(order);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["orders"] = orders;
        response["total"] = (int)orders.size();
        auto resp = HttpResponse::newHttpJsonResponse(response); callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}
