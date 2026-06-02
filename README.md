# Freelance Marketplace

Маркетплейс фрилансеров с веб-краулером. Стек: C++ Drogon + React + PostgreSQL.


## Быстрый запуск

git clone https://github.com/Programm344/freelance-marketplace.git

cd freelance-marketplace

make all

Открыть http://localhost:3000


## Пошаговый запуск

make install

make db

make build

make run


## Аккаунты

Админ: admin@freelance.ru / 123456

Модератор: moder@freelance.ru / 123456

Фрилансер: ivan@freelance.ru / 123456

Заказчик: company@freelance.ru / 123456


## Проверка функционала

1. Регистрация

2. Профиль фрилансера (ivan@freelance.ru)

3. Создать заказ (company@freelance.ru)

4. Модерация (moder@freelance.ru)

5. Отклик и чат

6. Отзывы

7. Краулер (admin@freelance.ru)

8. Статистика и CSV


## API

POST /api/auth/register — Регистрация

POST /api/auth/login — Вход (JWT)

GET /api/orders — Список заказов

POST /api/orders — Создать заказ

GET /api/orders/search — Поиск

POST /api/responses — Отклик

POST /api/responses/{id}/accept — Принять

GET /api/moderation/orders — Модерация

POST /api/crawler/start — Краулер

GET /api/admin/stats — Статистика

GET /api/admin/export/csv — CSV


## Функционал

4 роли, 9 статусов заказов, отклики, чат, отзывы, модерация, краулер, уведомления, CSV-экспорт, JWT, защита от SQL-инъекций
