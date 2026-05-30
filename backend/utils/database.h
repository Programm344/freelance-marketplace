#pragma once
#include <drogon/drogon.h>
#include <pqxx/pqxx>
#include <memory>
#include <string>

class Database {
private:
    Database() = default;
    std::unique_ptr<pqxx::connection> conn_;
    
public:
    static Database& getInstance() {
        static Database instance;
        return instance;
    }
    
    void initialize(const Json::Value& config) {
        std::string connStr = fmt::format(
            "host={} port={} dbname={} user={} password={}",
            config["host"].asString(),
            config["port"].asString(),
            config["dbname"].asString(),
            config["user"].asString(),
            config["password"].asString()
        );
        
        conn_ = std::make_unique<pqxx::connection>(connStr);
    }
    
    pqxx::connection& getConnection() {
        return *conn_;
    }
    
    template<typename... Args>
    pqxx::result execute(const std::string& query, Args&&... args) {
        pqxx::work txn(*conn_);
        auto result = txn.exec_params(query, std::forward<Args>(args)...);
        txn.commit();
        return result;
    }
};
