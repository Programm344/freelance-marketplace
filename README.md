# Freelance Marketplace

Маркетплейс фрилансеров с веб-краулером. Стек: C++ Drogon + React + PostgreSQL.

---

## Запуск (WSL / Ubuntu 22.04+)

### 1. Клонировать репозиторий
git clone https://github.com/Programm344/freelance-marketplace.git
cd freelance-marketplace

### 2. Установить PostgreSQL
sudo apt update
sudo apt install -y postgresql postgresql-client
sudo service postgresql start

### 3. Создать базу данных
sudo -u postgres psql -c "CREATE USER freelancer WITH PASSWORD 'freelancer_pass';"
sudo -u postgres psql -c "CREATE DATABASE freelance_marketplace OWNER freelancer;"
sudo -u postgres psql -d freelance_marketplace -c "GRANT ALL ON SCHEMA public TO freelancer;"
PGPASSWORD=freelancer_pass psql -h 127.0.0.1 -U freelancer -d freelance_marketplace -f database/migrations/001_initial_schema.sql
PGPASSWORD=freelancer_pass psql -h 127.0.0.1 -U freelancer -d freelance_marketplace -f database/migrations/002_seed_data.sql

### 4. Собрать и запустить бэкенд (терминал 1)
cd backend
mkdir -p build && cd build
cmake .. && make -j$(nproc)
cd ..
./build/freelance_backend

### 5. Запустить фронтенд (терминал 2)
cd frontend
npm install
npm start

### 6. Открыть
http://localhost:3000

---

## Аккаунты

Админ:     admin@freelance.ru  / 123456
Модератор: moder@freelance.ru  / 123456
Фрилансер: ivan@freelance.ru   / 123456
Заказчик:  company@freelance.ru / 123456

---

## Проверка функционала

1. Регистрация — http://localhost:3000/register
2. Профиль — войти как ivan@freelance.ru
3. Создать заказ — войти как company@freelance.ru
4. Модерация — войти как moder@freelance.ru
5. Отклик — фрилансер ищет заказ и откликается
6. Чат — заказчик принимает отклик
7. Отзывы — завершить заказ, оставить оценку
8. Краулер — админ запускает сбор заказов
9. Статистика и CSV — админ смотрит сводку

---

## API

POST /api/auth/register        — Регистрация
POST /api/auth/login            — Вход (JWT)
GET  /api/orders                — Список заказов
POST /api/orders                — Создать заказ
GET  /api/orders/search         — Поиск
POST /api/responses             — Отклик
POST /api/responses/{id}/accept — Принять
GET  /api/moderation/orders     — Модерация
POST /api/crawler/start         — Краулер
GET  /api/admin/stats           — Статистика
GET  /api/admin/export/csv      — CSV

## Функционал
4 роли, 9 статусов заказов, отклики, чат, отзывы, модерация,
краулер, уведомления, CSV-экспорт, JWT, защита от SQL-инъекций
