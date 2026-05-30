# Freelance Marketplace

Маркетплейс фрилансеров с веб-краулером. Стек: C++ Drogon + React + PostgreSQL.

---

## Как запустить (WSL / Ubuntu 22.04+)

### Шаг 1. Клонировать репозиторий
git clone https://github.com/Programm344/freelance-marketplace.git
cd freelance-marketplace

### Шаг 2. Установить PostgreSQL (один раз)
sudo apt update && sudo apt install -y postgresql postgresql-client && sudo service postgresql start

### Шаг 3. Создать базу данных (один раз)
sudo -u postgres psql -c "CREATE USER freelancer WITH PASSWORD 'freelancer_pass';" && sudo -u postgres psql -c "CREATE DATABASE freelance_marketplace OWNER freelancer;" && sudo -u postgres psql -d freelance_marketplace -c "GRANT ALL ON SCHEMA public TO freelancer;" && PGPASSWORD=freelancer_pass psql -h 127.0.0.1 -U freelancer -d freelance_marketplace -f database/migrations/001_initial_schema.sql && PGPASSWORD=freelancer_pass psql -h 127.0.0.1 -U freelancer -d freelance_marketplace -f database/migrations/002_seed_data.sql

### Шаг 4. Запустить бэкенд (терминал 1)
cd backend && chmod +x build/freelance_backend && ./build/freelance_backend

Сервер: http://localhost:8080

### Шаг 5. Запустить фронтенд (терминал 2)
cd frontend && npm install && npm start

Открыть: http://localhost:3000

### Шаг 6. Войти в систему

| Роль | Логин | Пароль |
|------|-------|--------|
| Админ | admin@freelance.ru | 123456 |
| Модератор | moder@freelance.ru | 123456 |
| Фрилансер | ivan@freelance.ru | 123456 |
| Заказчик | company@freelance.ru | 123456 |

---

## API

| Метод | Путь | Описание |
|-------|------|----------|
| POST | /api/auth/register | Регистрация |
| POST | /api/auth/login | Вход (JWT) |
| GET | /api/orders | Список заказов |
| POST | /api/orders | Создать заказ |
| GET | /api/orders/search | Поиск заказов |
| POST | /api/responses | Откликнуться |
| POST | /api/responses/{id}/accept | Принять отклик |
| GET | /api/moderation/orders | Модерация |
| POST | /api/crawler/start | Запуск краулера |
| GET | /api/admin/stats | Статистика |
| GET | /api/admin/export/csv | CSV экспорт |

## Функционал
- 4 роли: фрилансер, заказчик, модератор, админ
- CRUD заказов с 9 статусами
- Отклики, чат, отзывы, рейтинги
- Модерация заказов и заявок на роль
- Веб-краулер (Freelance.ru, Habr Freelance)
- Статистика и CSV-экспорт
- JWT-авторизация
- Защита от SQL-инъекций
