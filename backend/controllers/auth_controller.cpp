#include "auth_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include <openssl/evp.h>
#include "../utils/jwt_helper.h"
#include <sstream>
#include <iomanip>

using namespace drogon;

std::string hashPassword(const std::string& password) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, password.c_str(), password.length());
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
    std::stringstream ss;
    for(unsigned int i = 0; i < hash_len; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

void AuthController::register_user(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    if (!json) {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value{{"error", "Invalid JSON"}});
        resp->setStatusCode(k400BadRequest); callback(resp); return;
    }
    
    std::string email = (*json)["email"].asString();
    std::string password = (*json)["password"].asString();
    if (email.empty() || password.empty()) {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value{{"error", "Email and password required"}});
        resp->setStatusCode(k400BadRequest); callback(resp); return;
    }
    
    try {
        auto db = app().getDbClient("default");
        auto existing = db->execSqlSync("SELECT id FROM users WHERE email = $1", email);
        if (existing.size() > 0) {
            auto resp = HttpResponse::newHttpJsonResponse(Json::Value{{"error", "Email already exists"}});
            resp->setStatusCode(k400BadRequest); callback(resp); return;
        }
        
        std::string hashed = hashPassword(password);
        auto result = db->execSqlSync(
            "INSERT INTO users (email, password_hash, role) VALUES ($1, $2, 'freelancer') RETURNING id, email, role",
            email, hashed
        );
        
        int userId = result[0]["id"].as<int>();
        std::string role = result[0]["role"].as<std::string>();
        db->execSqlSync("INSERT INTO freelancer_profiles (user_id) VALUES ($1)", userId);
        
        std::string token = JwtHelper::generateToken(userId, role, email);
        
        Json::Value resp;
        resp["status"] = "success";
        resp["user_id"] = userId;
        resp["email"] = email;
        resp["role"] = role;
        resp["token"] = token;
        auto httpResp = HttpResponse::newHttpJsonResponse(resp);
        httpResp->setStatusCode(k201Created); callback(httpResp);
    } catch (const std::exception& e) {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value{{"error", e.what()}});
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void AuthController::login(const HttpRequestPtr &req,
                          std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    std::string email = (*json)["email"].asString();
    std::string password = (*json)["password"].asString();
    
    try {
        auto db = app().getDbClient("default");
        auto result = db->execSqlSync(
            "SELECT id, email, password_hash, role, is_blocked FROM users WHERE email = $1", email
        );
        if (result.size() == 0) {
            auto resp = HttpResponse::newHttpJsonResponse(Json::Value{{"error", "Invalid credentials"}});
            resp->setStatusCode(k401Unauthorized); callback(resp); return;
        }
        
        auto row = result[0];
        if (row["is_blocked"].as<bool>()) {
            auto resp = HttpResponse::newHttpJsonResponse(Json::Value{{"error", "User blocked"}});
            resp->setStatusCode(k403Forbidden); callback(resp); return;
        }
        if (hashPassword(password) != row["password_hash"].as<std::string>()) {
            auto resp = HttpResponse::newHttpJsonResponse(Json::Value{{"error", "Invalid credentials"}});
            resp->setStatusCode(k401Unauthorized); callback(resp); return;
        }
        
        int userId = row["id"].as<int>();
        std::string role = row["role"].as<std::string>();
        std::string token = JwtHelper::generateToken(userId, role, email);
        
        Json::Value resp;
        resp["status"] = "success";
        resp["user_id"] = userId;
        resp["email"] = email;
        resp["role"] = role;
        resp["token"] = token;
        auto httpResp = HttpResponse::newHttpJsonResponse(resp); callback(httpResp);
    } catch (const std::exception& e) {
        auto resp = HttpResponse::newHttpJsonResponse(Json::Value{{"error", e.what()}});
        resp->setStatusCode(k500InternalServerError); callback(resp);
    }
}

void AuthController::test(const HttpRequestPtr &req,
                         std::function<void(const HttpResponsePtr &)> &&callback) {
    Json::Value resp;
    resp["status"] = "ok";
    resp["message"] = "Backend running";
    auto httpResp = HttpResponse::newHttpJsonResponse(resp); callback(httpResp);
}
