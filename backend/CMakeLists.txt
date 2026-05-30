cmake_minimum_required(VERSION 3.16)
project(freelance_marketplace_backend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

add_executable(freelance_backend
    main.cpp
    controllers/auth_controller.cpp
    controllers/user_controller.cpp
    controllers/order_controller.cpp
    controllers/response_controller.cpp
    controllers/moderation_controller.cpp
    controllers/complaint_controller.cpp
    controllers/review_controller.cpp
    controllers/crawler_controller.cpp
    controllers/message_controller.cpp
    controllers/search_controller.cpp
    controllers/notification_controller.cpp
    controllers/admin_controller.cpp
    controllers/role_controller.cpp
    crawler/web_crawler.cpp
    utils/jwt_helper.cpp
    middleware/auth_middleware.cpp
)

target_link_libraries(freelance_backend 
    PRIVATE 
    Drogon::Drogon
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
)
