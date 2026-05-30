#include <drogon/drogon.h>

using namespace drogon;

int main() {
    app().loadConfigFile("config/config.json");
    
    // CORS - разрешаем всё
    app().registerPostHandlingAdvice([](const HttpRequestPtr &req,
                                         const HttpResponsePtr &resp) {
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, PATCH");
        resp->addHeader("Access-Control-Allow-Headers", "*");
        resp->addHeader("Access-Control-Max-Age", "86400");
        
        if (req->method() == Options) {
            resp->setStatusCode(k200OK);
            resp->setContentTypeCode(CT_NONE);
        }
    });
    
    LOG_INFO << "Server running on http://localhost:8080";
    
    app().run();
    return 0;
}
