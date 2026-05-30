#pragma once
#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>
#include "../models/user.h"

class AuthService {
public:
    AuthService() = default;
    
    Json::Value registerUser(const drogon::orm::DbClientPtr& db,
                             const std::string& email, 
                             const std::string& password);
    
    Json::Value loginUser(const drogon::orm::DbClientPtr& db,
                          const std::string& email, 
                          const std::string& password);
    
private:
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);
    std::string generateToken(const User& user);
};
