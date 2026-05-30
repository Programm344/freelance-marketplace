#pragma once
#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>

class OrderService {
public:
    OrderService() = default;
    
    Json::Value getOrders(const drogon::orm::DbClientPtr& db,
                          const std::map<std::string, std::string>& filters);
    
    Json::Value getOrderById(const drogon::orm::DbClientPtr& db, int id);
    
    Json::Value createOrder(const drogon::orm::DbClientPtr& db,
                            const Json::Value& data, int userId);
};
