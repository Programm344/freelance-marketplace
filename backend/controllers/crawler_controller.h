#pragma once
#include <drogon/HttpController.h>
#include "../crawler/web_crawler.h"

class CrawlerController : public drogon::HttpController<CrawlerController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CrawlerController::start_crawl, "/api/crawler/start", drogon::Post);
    ADD_METHOD_TO(CrawlerController::get_sources, "/api/crawler/sources", drogon::Get);
    ADD_METHOD_TO(CrawlerController::add_source, "/api/crawler/sources", drogon::Post);
    ADD_METHOD_TO(CrawlerController::get_external_orders, "/api/crawler/orders", drogon::Get);
    ADD_METHOD_TO(CrawlerController::get_logs, "/api/crawler/logs", drogon::Get);
    METHOD_LIST_END
    
    void start_crawl(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_sources(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void add_source(const drogon::HttpRequestPtr &req,
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_external_orders(const drogon::HttpRequestPtr &req,
                            std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    void get_logs(const drogon::HttpRequestPtr &req,
                 std::function<void(const drogon::HttpResponsePtr &)> &&callback);
    
private:
    WebCrawler crawler_;
};
