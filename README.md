# Freelance Marketplace

Маркетплейс фрилансеров с веб-краулером. Стек: C++ Drogon + React + PostgreSQL.


## Запуск

git clone https://github.com/Programm344/freelance-marketplace.git
cd freelance-marketplace
make all

Открыть http://localhost:3000


## Аккаунты

Админ:     admin@freelance.ru  / 123456
Модератор: moder@freelance.ru  / 123456
Фрилансер: ivan@freelance.ru   / 123456
Заказчик:  company@freelance.ru / 123456


## Проверка

1. Регистрация — выбрать роль при регистрации
2. Профиль — заполнить обязательно
3. Заказчик — заполнить компанию, отправить заявку
4. Модератор — одобрить заявку и заказы
5. Фрилансер — найти заказ, откликнуться
6. Чат — между заказчиком и фрилансером
7. Отзывы — после завершения заказа
8. Краулер — админ запускает сбор
9. Статистика и CSV — админ/модератор


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
