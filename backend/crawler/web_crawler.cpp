#include "web_crawler.h"
#include <drogon/orm/DbClient.h>
#include "../utils/helpers.h"
#include <curl/curl.h>
#include <regex>
#include <set>
#include <algorithm>

using namespace drogon;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}

WebCrawler::WebCrawler() { curl_global_init(CURL_GLOBAL_ALL); }
WebCrawler::~WebCrawler() { curl_global_cleanup(); }

std::string fetchPage(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return response;
}

std::string stripHtml(const std::string& html) {
    std::string s = std::regex_replace(html, std::regex("<[^>]*>"), " ");
    s = std::regex_replace(s, std::regex("&[a-z]+;"), " ");
    s = std::regex_replace(s, std::regex("\\s+"), " ");
    size_t a = s.find_first_not_of(" \t\n\r");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\n\r");
    return s.substr(a, b - a + 1);
}

double extractBudget(const std::string& text) {
    std::regex re("(\\d[\\d\\s]{0,10})\\s*(?:₽|руб|р\\.)", std::regex::icase);
    std::smatch m;
    if (std::regex_search(text, m, re)) {
        std::string num = std::regex_replace(m[1].str(), std::regex("\\s"), "");
        try { return std::stod(num); } catch (...) {}
    }
    return 0;
}

// Список СТОП-СЛОВ — если заголовок содержит любое из них → МУСОР

std::vector<CrawlResult> WebCrawler::crawlSource(const Json::Value& sourceConfig) {
    std::string baseUrl = sourceConfig["base_url"].asString();
    std::string sourceName = sourceConfig["name"].asString();
    LOG_INFO << "Crawling: " << sourceName;
    
    std::string html = fetchPage(baseUrl);
    if (html.empty()) {
        LOG_WARN << "Empty response from " << baseUrl;
        return {};
    }
    
    std::vector<CrawlResult> results;
    std::set<std::string> seen;
    
    // Парсим все ссылки
    std::regex linkRegex("<a[^>]*href=\"([^\"]*)\"[^>]*>([^<]{20,300})</a>");
    std::sregex_iterator it(html.begin(), html.end(), linkRegex);
    std::sregex_iterator end;
    
    while (it != end && results.size() < 20) {
        std::string linkUrl = (*it)[1].str();
        std::string linkText = stripHtml((*it)[2].str());
        
        // Простой фильтр мусора (кириллица!)
        std::string lower = linkText;
        for (char& c : lower) {
            if (c >= 'А' && c <= 'Я') c += 32;        // А-Я → а-я
            if (c == 'Ё') c = 'ё';
            if (c >= 'A' && c <= 'Z') c += 32;        // A-Z → a-z
        }
        bool skip = false;
        std::vector<std::string> bad = {"каталог", "отказ от", "политика конфиденц", "дисклеймер", "партнер", "cookie", "разместить задание", "о причинах", "сбросить поиск", "виртуальный hr", "умный поиск", "найти работу"};
        for (const auto& b : bad) {
            if (lower.find(b) != std::string::npos) { skip = true; break; }
        }
        
        if (!skip && seen.find(linkUrl) == seen.end()) {
            if (linkUrl.find("http") != 0) {
                if (linkUrl[0] == '/') {
                    size_t pos = baseUrl.find('/', 8);
                    std::string domain = (pos != std::string::npos) ? baseUrl.substr(0, pos) : baseUrl;
                    linkUrl = domain + linkUrl;
                }
            }
            
            CrawlResult r;
            r.title = linkText;
            r.description = "Собрано с " + sourceName;
            r.category = "Не указана";
            r.skills = "Не указаны";
            r.budget = extractBudget(linkText);
            r.deadline = "";
            r.source_url = linkUrl;
            r.source_name = sourceName;
            
            results.push_back(r);
            seen.insert(linkUrl);
        }
        ++it;
    }
    
    LOG_INFO << sourceName << ": " << results.size() << " items after filtering";
    return results;
}


void WebCrawler::notifyMatchingSearches(const orm::DbClientPtr& db, const std::vector<CrawlResult>& results) {
    if (results.empty()) return;
    
    auto users = db->execSqlSync("SELECT DISTINCT user_id FROM saved_searches");
    
    for (const auto& user : users) {
        int userId = user["user_id"].as<int>();
        auto searches = db->execSqlSync("SELECT * FROM saved_searches WHERE user_id = $1", userId);
        
        for (const auto& r : results) {
            for (const auto& s : searches) {
                std::string keywords = s["keywords"].as<std::string>();
                double budgetMin = s["budget_min"].isNull() ? 0 : s["budget_min"].as<double>();
                double budgetMax = s["budget_max"].isNull() ? 999999999 : s["budget_max"].as<double>();
                
                bool budgetMatch = (r.budget == 0) || (r.budget >= budgetMin && r.budget <= budgetMax);
                bool keywordMatch = keywords.empty() || 
                    r.title.find(keywords) != std::string::npos ||
                    r.description.find(keywords) != std::string::npos;
                
                if (budgetMatch && keywordMatch) {
                    db->execSqlSync(
                        "INSERT INTO notifications (user_id, type, title, message) "
                        "VALUES ($1, 'matching_order', 'Подходящий внешний заказ', "
                        "'По поиску \"' || $2 || '\" найден: ' || $3)",
                        userId, s["name"].as<std::string>(), r.title
                    );
                    break; // одно уведомление на заказ
                }
            }
        }
    }
    LOG_INFO << "Notifications sent for matching searches";
}





void WebCrawler::saveResults
(const orm::DbClientPtr& db, const std::vector<CrawlResult>& results) {
    for (const auto& result : results) {
        // Проверка уже сделана в crawlSource
        
        std::string pgSkills = toPgArray(result.skills);
        auto existing = db->execSqlSync("SELECT id FROM external_orders WHERE source_url = $1", result.source_url);
        
        if (existing.size() == 0) {
            db->execSqlSync(
                "INSERT INTO external_orders (title, description, category, skills, budget, deadline, source_url, source_name, status) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, 'new')",
                result.title, result.description, result.category,
                pgSkills, result.budget, result.deadline,
                result.source_url, result.source_name
            );
        }
    }
    // Автоочистка мусора
    db->execSqlSync("DELETE FROM external_orders WHERE title ILIKE '%каталог%' OR title ILIKE '%отказ от%' OR title ILIKE '%политика конфиденц%' OR title ILIKE '%дисклеймер%' OR title ILIKE '%разместить задание%' OR title ILIKE '%сбросить поиск%' OR title ILIKE '%виртуальный hr%' OR title ILIKE '%умный поиск%' OR title ILIKE '%о причинах%'");
}
