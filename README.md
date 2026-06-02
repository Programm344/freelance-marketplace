# Freelance Marketplace

Маркетплейс фрилансеров с веб-краулером. Стек: C++ Drogon + React + PostgreSQL.


## Запуск

### 1. Установить PostgreSQL (один раз)
sudo apt update && sudo apt install -y postgresql postgresql-client
sudo service postgresql start

### 2. Распаковать архив и запустить
tar -xzf freelance_deploy.tar.gz
cd freelance_deploy
./run.sh

### 3. Открыть в браузере
http://localhost:8080


## Аккаунты

Админ:     admin@freelance.ru / 123456
Модератор: moder@freelance.ru / 123456
Фрилансер: ivan@freelance.ru  / 123456
Заказчик:  company@freelance.ru / 123456


## Проверка функционала

1. Регистрация — открыть http://localhost:8080/register
2. Профиль фрилансера — войти как ivan@freelance.ru, заполнить навыки
3. Создать заказ — войти как company@freelance.ru, создать заказ
4. Модерация — войти как moder@freelance.ru, одобрить заказ
5. Отклик — фрилансер ищет заказ и откликается
6. Чат — заказчик принимает отклик, появляется чат
7. Отзывы — завершить заказ, оставить оценку
8. Краулер — админ запускает сбор заказов с бирж
9. Статистика — админ смотрит сводку, скачивает CSV


## API

POST /api/auth/register         — Регистрация
POST /api/auth/login             — Вход (JWT)
GET  /api/orders                 — Список заказов
POST /api/orders                 — Создать заказ
GET  /api/orders/search          — Поиск
POST /api/responses              — Отклик
POST /api/responses/{id}/accept  — Принять отклик
GET  /api/moderation/orders      — Модерация
POST /api/crawler/start          — Краулер
GET  /api/admin/stats            — Статистика
GET  /api/admin/export/csv       — CSV


## Функционал

- 4 роли с разграничением прав
- CRUD заказов с 9 статусами
- Отклики, чат, отзывы, рейтинги
- Модерация заказов и заявок
- Веб-краулер (Freelance.ru, Habr Freelance)
- Сохраненные поиски и уведомления
- Статистика и CSV-экспорт
- JWT-авторизация, защита от SQL-инъекций
