#include "../utils/request_helper.h"
#include "user_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include "../utils/helpers.h"

using namespace drogon;

void UserController::get_profile(const HttpRequestPtr &req,
                                 std::function<void(const HttpResponsePtr &)> &&callback) {
    auto params = req->getParameters();
    int userId = getUserIdFromRequest(req);
    if (params.find("user_id") != params.end()) {
        userId = std::stoi(params["user_id"]);
    }
    
    try {
        auto dbClient = app().getDbClient("default");
        auto userResult = dbClient->execSqlSync(
            "SELECT id, email, role, is_blocked, created_at FROM users WHERE id = $1", userId
        );
        
        if (userResult.size() == 0) {
            Json::Value error; error["error"] = "User not found";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound); callback(resp); return;
        }
        
        auto userRow = userResult[0];
        Json::Value profile;
        profile["id"] = userRow["id"].as<int>();
        profile["email"] = userRow["email"].as<std::string>();
        profile["role"] = userRow["role"].as<std::string>();
        profile["created_at"] = userRow["created_at"].as<std::string>();
        
        if (userRow["role"].as<std::string>() == "freelancer") {
            auto fr = dbClient->execSqlSync("SELECT * FROM freelancer_profiles WHERE user_id = $1", userId);
            if (fr.size() > 0) {
                auto row = fr[0];
                profile["display_name"] = row["display_name"].isNull() ? "" : row["display_name"].as<std::string>();
                profile["specialization"] = row["specialization"].isNull() ? "" : row["specialization"].as<std::string>();
                profile["experience"] = row["experience_description"].isNull() ? "" : row["experience_description"].as<std::string>();
                profile["skills"] = row["skills"].isNull() ? "" : row["skills"].as<std::string>();
                profile["hourly_rate"] = row["hourly_rate"].isNull() ? 0 : row["hourly_rate"].as<double>();
                profile["rating"] = row["rating"].isNull() ? 0.0 : row["rating"].as<double>();
                profile["is_available"] = row["is_available"].isNull() ? true : row["is_available"].as<bool>();
            }
        }
        
        if (userRow["role"].as<std::string>() == "customer") {
            auto cp = dbClient->execSqlSync("SELECT * FROM customer_profiles WHERE user_id = $1", userId);
            if (cp.size() > 0) {
                auto row = cp[0];
                profile["company_name"] = row["company_name"].isNull() ? "" : row["company_name"].as<std::string>();
                profile["company_description"] = row["description"].isNull() ? "" : row["description"].as<std::string>();
                profile["rating"] = row["rating"].isNull() ? 0.0 : row["rating"].as<double>();
            }
        }
        
        Json::Value response;
        response["status"] = "success";
        response["profile"] = profile;
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void UserController::update_profile(const HttpRequestPtr &req,
                                    std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    auto params = req->getParameters();
    int userId = getUserIdFromRequest(req);
    if (params.find("user_id") != params.end()) {
        userId = std::stoi(params["user_id"]);
    }
    
    try {
        auto dbClient = app().getDbClient("default");
        std::string skills = (*json)["skills"].asString();
        
        if (!skills.empty()) {
            std::string pgSkills = toPgArray(skills);
            dbClient->execSqlSync(
                "UPDATE freelancer_profiles SET skills = $1, updated_at = NOW() WHERE user_id = $2",
                pgSkills, userId
            );
        }
        
        dbClient->execSqlSync(
            "UPDATE freelancer_profiles SET display_name = COALESCE(NULLIF($1, ''), display_name), "
            "specialization = COALESCE(NULLIF($2, ''), specialization), "
            "experience_description = COALESCE(NULLIF($3, ''), experience_description), "
            "hourly_rate = COALESCE($4, hourly_rate), is_available = COALESCE($5, is_available), "
            "updated_at = NOW() WHERE user_id = $6",
            (*json)["display_name"].asString(), (*json)["specialization"].asString(),
            (*json)["experience"].asString(), (*json)["hourly_rate"].asDouble(),
            (*json)["is_available"].asBool(), userId
        );
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Profile updated";
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void UserController::get_freelancer_profile(const HttpRequestPtr &req,
                                            std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT u.id, u.email, fp.display_name, fp.specialization, fp.experience_description, "
            "fp.skills, fp.hourly_rate, fp.rating, fp.is_available "
            "FROM users u JOIN freelancer_profiles fp ON u.id = fp.user_id "
            "WHERE u.id = $1 AND u.role = 'freelancer'", id
        );
        
        if (result.size() == 0) {
            Json::Value error; error["error"] = "Freelancer not found";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound); callback(resp); return;
        }
        
        auto row = result[0];
        Json::Value profile;
        profile["id"] = row["id"].as<int>();
        profile["display_name"] = row["display_name"].isNull() ? "" : row["display_name"].as<std::string>();
        profile["specialization"] = row["specialization"].isNull() ? "" : row["specialization"].as<std::string>();
        profile["skills"] = row["skills"].isNull() ? "" : row["skills"].as<std::string>();
        profile["hourly_rate"] = row["hourly_rate"].isNull() ? 0 : row["hourly_rate"].as<double>();
        profile["rating"] = row["rating"].isNull() ? 0.0 : row["rating"].as<double>();
        profile["is_available"] = row["is_available"].isNull() ? true : row["is_available"].as<bool>();
        
        Json::Value response;
        response["status"] = "success";
        response["profile"] = profile;
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void UserController::get_customer_profile(const HttpRequestPtr &req,
                                          std::function<void(const HttpResponsePtr &)> &&callback, int id) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT u.id, cp.company_name, cp.description, cp.rating "
            "FROM users u JOIN customer_profiles cp ON u.id = cp.user_id "
            "WHERE u.id = $1", id
        );
        
        if (result.size() == 0) {
            Json::Value error; error["error"] = "Customer not found";
            auto resp = HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(k404NotFound); callback(resp); return;
        }
        
        auto row = result[0];
        Json::Value profile;
        profile["id"] = row["id"].as<int>();
        profile["company_name"] = row["company_name"].isNull() ? "" : row["company_name"].as<std::string>();
        profile["description"] = row["description"].isNull() ? "" : row["description"].as<std::string>();
        profile["rating"] = row["rating"].isNull() ? 0.0 : row["rating"].as<double>();
        
        Json::Value response;
        response["status"] = "success";
        response["profile"] = profile;
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void UserController::search_freelancers(const HttpRequestPtr &req,
                                        std::function<void(const HttpResponsePtr &)> &&callback) {
    auto params = req->getParameters();
    try {
        auto dbClient = app().getDbClient("default");
        
        std::string skill = params.find("skill") != params.end() ? params["skill"] : "";
        bool available = params.find("available") != params.end() && params["available"] == "true";
        
        std::string query = 
            "SELECT u.id, fp.display_name, fp.specialization, fp.skills, "
            "fp.hourly_rate, fp.rating, fp.is_available "
            "FROM users u JOIN freelancer_profiles fp ON u.id = fp.user_id "
            "WHERE u.role = 'freelancer' AND u.is_blocked = false";
        
        if (!skill.empty())
            query += " AND fp.skills::text ILIKE $1";
        if (available)
            query += " AND fp.is_available = true";
        
        query += " ORDER BY fp.rating DESC LIMIT 50";
        
        auto result = skill.empty() ?
            dbClient->execSqlSync(query) :
            dbClient->execSqlSync(query, "%" + skill + "%");
        
        Json::Value freelancers(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value f;
            f["id"] = row["id"].as<int>();
            f["display_name"] = row["display_name"].isNull() ? "" : row["display_name"].as<std::string>();
            f["specialization"] = row["specialization"].isNull() ? "" : row["specialization"].as<std::string>();
            f["skills"] = row["skills"].isNull() ? "" : row["skills"].as<std::string>();
            f["hourly_rate"] = row["hourly_rate"].isNull() ? 0 : row["hourly_rate"].as<double>();
            f["rating"] = row["rating"].isNull() ? 0.0 : row["rating"].as<double>();
            f["is_available"] = row["is_available"].isNull() ? true : row["is_available"].as<bool>();
            freelancers.append(f);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["freelancers"] = freelancers;
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
    } catch (const std::exception& e) {
        Json::Value error; error["error"] = e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}
