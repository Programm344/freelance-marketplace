#include "auth_service.h"
#include <openssl/evp.h>
#include <sstream>
#include <iomanip>

using namespace drogon;

std::string AuthService::hashPassword(const std::string& password) {
    // Новый API OpenSSL 3.0 (EVP)
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, password.c_str(), password.length());
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
    
    std::stringstream ss;
    for(unsigned int i = 0; i < hash_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool AuthService::verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

std::string AuthService::generateToken(const User& user) {
    // Заглушка JWT токена
    Json::Value payload;
    payload["user_id"] = user.getId();
    payload["email"] = user.getEmail();
    payload["role"] = user.getRole();
    payload["exp"] = (int)time(nullptr) + 3600;
    
    Json::FastWriter writer;
    return "jwt_" + writer.write(payload);
}

Json::Value AuthService::registerUser(const orm::DbClientPtr& db,
                                      const std::string& email,
                                      const std::string& password) {
    Json::Value response;
    
    try {
        // Проверяем существующего пользователя напрямую через SQL
        auto existing = db->execSqlSync(
            "SELECT id FROM users WHERE email = $1", email
        );
        
        if (existing.size() > 0) {
            response["error"] = "User with this email already exists";
            response["status"] = "error";
            return response;
        }
        
        // Хешируем пароль и создаём пользователя
        std::string hashedPassword = hashPassword(password);
        auto result = db->execSqlSync(
            "INSERT INTO users (email, password_hash, role) "
            "VALUES ($1, $2, 'freelancer') RETURNING id, email, role",
            email, hashedPassword
        );
        
        if (result.size() > 0) {
            auto row = result[0];
            User user;
            user.setId(row["id"].as<int>());
            user.setEmail(row["email"].as<std::string>());
            user.setRole(row["role"].as<std::string>());
            
            // Создаём профиль фрилансера
            db->execSqlSync(
                "INSERT INTO freelancer_profiles (user_id) VALUES ($1)",
                user.getId()
            );
            
            // Генерируем токен
            std::string token = generateToken(user);
            
            response["status"] = "success";
            response["message"] = "User registered successfully";
            response["user"] = user.toJson();
            response["token"] = token;
        }
        
    } catch (const std::exception& e) {
        response["error"] = std::string("Registration failed: ") + e.what();
        response["status"] = "error";
    }
    
    return response;
}

Json::Value AuthService::loginUser(const orm::DbClientPtr& db,
                                   const std::string& email,
                                   const std::string& password) {
    Json::Value response;
    
    try {
        auto result = db->execSqlSync(
            "SELECT id, email, password_hash, role, is_blocked FROM users WHERE email = $1",
            email
        );
        
        if (result.size() == 0) {
            response["error"] = "Invalid email or password";
            response["status"] = "error";
            return response;
        }
        
        auto row = result[0];
        
        if (row["is_blocked"].as<bool>()) {
            response["error"] = "User is blocked";
            response["status"] = "error";
            return response;
        }
        
        std::string storedHash = row["password_hash"].as<std::string>();
        if (!verifyPassword(password, storedHash)) {
            response["error"] = "Invalid email or password";
            response["status"] = "error";
            return response;
        }
        
        User user;
        user.setId(row["id"].as<int>());
        user.setEmail(row["email"].as<std::string>());
        user.setRole(row["role"].as<std::string>());
        
        std::string token = generateToken(user);
        
        response["status"] = "success";
        response["message"] = "Login successful";
        response["user"] = user.toJson();
        response["token"] = token;
        
    } catch (const std::exception& e) {
        response["error"] = std::string("Login failed: ") + e.what();
        response["status"] = "error";
    }
    
    return response;
}
