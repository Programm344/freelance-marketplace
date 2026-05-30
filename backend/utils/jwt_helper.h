#pragma once
#include <string>
#include <drogon/drogon.h>

class JwtHelper {
public:
    static std::string generateToken(int userId, const std::string& role, const std::string& email);
    static bool verifyToken(const std::string& token, int& userId, std::string& role);
    
private:
    static const std::string SECRET;
};
