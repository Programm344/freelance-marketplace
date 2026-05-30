#pragma once
#include <string>
#include <drogon/orm/DbClient.h>

class User {
public:
    User() = default;
    
    // Getters
    int getId() const { return id_; }
    std::string getEmail() const { return email_; }
    std::string getPasswordHash() const { return password_hash_; }
    std::string getRole() const { return role_; }
    bool isBlocked() const { return is_blocked_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setEmail(const std::string& email) { email_ = email; }
    void setPasswordHash(const std::string& hash) { password_hash_ = hash; }
    void setRole(const std::string& role) { role_ = role; }
    void setBlocked(bool blocked) { is_blocked_ = blocked; }
    
    // Serialization
    Json::Value toJson() const {
        Json::Value json;
        json["id"] = id_;
        json["email"] = email_;
        json["role"] = role_;
        json["is_blocked"] = is_blocked_;
        json["created_at"] = created_at_;
        return json;
    }
    
private:
    int id_;
    std::string email_;
    std::string password_hash_;
    std::string role_;
    bool is_blocked_ = false;
    std::string created_at_;
    std::string updated_at_;
};
