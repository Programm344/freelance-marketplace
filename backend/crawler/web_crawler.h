#pragma once
#include <string>
#include <vector>
#include <drogon/drogon.h>

struct CrawlResult {
    std::string title;
    std::string description;
    std::string category;
    std::string skills;
    double budget = 0;
    std::string deadline;
    std::string source_url;
    std::string source_name;
};

class WebCrawler {
public:
    WebCrawler();
    ~WebCrawler();
    std::vector<CrawlResult> crawlSource(const Json::Value& sourceConfig);
    void saveResults(const drogon::orm::DbClientPtr& db, const std::vector<CrawlResult>& results);
    void notifyMatchingSearches(const drogon::orm::DbClientPtr& db, const std::vector<CrawlResult>& results);
};
