#include "crawler_controller.h"
#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>

using namespace drogon;

void CrawlerController::start_crawl(const HttpRequestPtr &req,
                                    std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        
        // Получаем активные источники
        auto sources = dbClient->execSqlSync(
            "SELECT * FROM crawler_sources WHERE status = 'active'"
        );
        
        int totalFound = 0;
        
        for (const auto& source : sources) {
            Json::Value sourceConfig;
            sourceConfig["id"] = source["id"].as<int>();
            sourceConfig["name"] = source["name"].as<std::string>();
            sourceConfig["base_url"] = source["base_url"].as<std::string>();
            
            auto startTime = std::chrono::steady_clock::now();
            
            try {
                auto results = crawler_.crawlSource(sourceConfig);
                crawler_.saveResults(dbClient, results);
                crawler_.notifyMatchingSearches(dbClient, results);
                // Считаем реальное количество после очистки
                auto cnt = dbClient->execSqlSync("SELECT COUNT(*) as c FROM external_orders WHERE source_name = $1 AND status = 'new'", source["name"].as<std::string>());
                int realFound = cnt[0]["c"].as<int>();
                totalFound += realFound;
                
                auto endTime = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
                
                // Логируем успешный запуск
                dbClient->execSqlSync(
                    "INSERT INTO crawl_logs (source_id, status, items_found, items_new, started_at, finished_at) "
                    "VALUES ($1, 'success', $2, $3, NOW(), NOW())",
                    source["id"].as<int>(), realFound, realFound
                );
                
                // Обновляем время последнего запуска
                dbClient->execSqlSync(
                    "UPDATE crawler_sources SET last_run_at = NOW() WHERE id = $1",
                    source["id"].as<int>()
                );
                
            } catch (const std::exception& e) {
                dbClient->execSqlSync(
                    "INSERT INTO crawl_logs (source_id, status, error_message, started_at, finished_at) "
                    "VALUES ($1, 'error', $2, NOW(), NOW())",
                    source["id"].as<int>(), e.what()
                );
            }
        }
        
        Json::Value response;
        response["status"] = "success";
        response["message"] = "Crawl completed";
        response["sources_processed"] = (int)sources.size();
        // Считаем реальное количество после очистки
    auto countResult = dbClient->execSqlSync("SELECT COUNT(*) as c FROM external_orders WHERE status = 'new'");
    int realCount = countResult[0]["c"].as<int>();
    response["items_found"] = realCount;
    response["total_in_db"] = realCount;
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Crawl failed: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void CrawlerController::get_sources(const HttpRequestPtr &req,
                                    std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync("SELECT * FROM crawler_sources ORDER BY created_at DESC");
        
        Json::Value sources(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value source;
            source["id"] = row["id"].as<int>();
            source["name"] = row["name"].as<std::string>();
            source["base_url"] = row["base_url"].as<std::string>();
            source["status"] = row["status"].as<std::string>();
            source["frequency"] = row["frequency"].as<int>();
            source["last_run_at"] = row["last_run_at"].isNull() ? "" : row["last_run_at"].as<std::string>();
            sources.append(source);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["sources"] = sources;
        
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

void CrawlerController::add_source(const HttpRequestPtr &req,
                                   std::function<void(const HttpResponsePtr &)> &&callback) {
    auto json = req->getJsonObject();
    
    try {
        auto dbClient = app().getDbClient("default");
        
        auto result = dbClient->execSqlSync(
            "INSERT INTO crawler_sources (name, base_url, crawl_rules, extract_rules, frequency) "
            "VALUES ($1, $2, $3, $4, $5) RETURNING id",
            (*json)["name"].asString(),
            (*json)["base_url"].asString(),
            (*json)["crawl_rules"].asString(),
            (*json)["extract_rules"].asString(),
            (*json)["frequency"].asInt()
        );
        
        Json::Value response;
        response["status"] = "success";
        response["source_id"] = result[0]["id"].as<int>();
        
        auto resp = HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(k201Created);
        callback(resp);
        
    } catch (const std::exception& e) {
        Json::Value error;
        error["error"] = std::string("Failed: ") + e.what();
        auto resp = HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(k500InternalServerError);
        callback(resp);
    }
}

void CrawlerController::get_external_orders(const HttpRequestPtr &req,
                                            std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT * FROM external_orders ORDER BY discovered_at DESC LIMIT 50"
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
            order["status"] = row["status"].as<std::string>();
            order["discovered_at"] = row["discovered_at"].as<std::string>();
            orders.append(order);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["orders"] = orders;
        
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

void CrawlerController::get_logs(const HttpRequestPtr &req,
                                 std::function<void(const HttpResponsePtr &)> &&callback) {
    try {
        auto dbClient = app().getDbClient("default");
        auto result = dbClient->execSqlSync(
            "SELECT cl.*, cs.name as source_name FROM crawl_logs cl "
            "JOIN crawler_sources cs ON cl.source_id = cs.id "
            "ORDER BY cl.started_at DESC LIMIT 50"
        );
        
        Json::Value logs(Json::arrayValue);
        for (const auto& row : result) {
            Json::Value log;
            log["id"] = row["id"].as<int>();
            log["source_name"] = row["source_name"].as<std::string>();
            log["status"] = row["status"].as<std::string>();
            log["items_found"] = row["items_found"].as<int>();
            log["error_message"] = row["error_message"].isNull() ? "" : row["error_message"].as<std::string>();
            log["started_at"] = row["started_at"].as<std::string>();
            logs.append(log);
        }
        
        Json::Value response;
        response["status"] = "success";
        response["logs"] = logs;
        
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
