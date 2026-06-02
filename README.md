# Freelance Marketplace

Маркетплейс фрилансеров с веб-краулером. Стек: C++ Drogon + React + PostgreSQL.

---

## Быстрый запуск (Docker)

### 1. Установить Docker
```bash
sudo apt update && sudo apt install -y docker.io docker-compose
2. Клонировать и запустить
git clone https://github.com/Programm344/freelance-marketplace.git
cd freelance-marketplace
docker-compose up
3. Открыть в браузере
http://localhost:8080
Аккаунты для входа
Роль	Логин	Пароль
Админ	admin@freelance.ru	123456
Модератор	moder@freelance.ru	123456
Фрилансер	ivan@freelance.ru	123456
Заказчик	company@freelance.ru	123456
Как проверить функционал
Регистрация — http://localhost:8080/register

Профиль — войти как ivan@freelance.ru, заполнить навыки и ставку

Создать заказ — войти как company@freelance.ru, создать заказ

Модерация — войти как moder@freelance.ru, одобрить заказ

Отклик — фрилансер ищет заказ и откликается

Чат — заказчик принимает отклик, появляется чат

Отзывы — завершить заказ, оставить оценку ★

Краулер — админ запускает сбор заказов с бирж

Статистика — админ/модератор смотрит сводку и CSV
API
Метод	Путь	Описание
POST	/api/auth/register	Регистрация
POST	/api/auth/login	Вход (JWT)
GET	/api/orders	Список заказов
POST	/api/orders	Создать заказ
GET	/api/orders/search	Поиск
POST	/api/responses	Отклик
GET	/api/moderation/orders	Модерация
POST	/api/crawler/start	Краулер
GET	/api/admin/stats	Статистика
GET	/api/admin/export/csv	CSV
Функционал
4 роли: фрилансер, заказчик, модератор, админ

CRUD заказов с 9 статусами

Отклики, чат, отзывы, рейтинги

Модерация заказов и заявок на роль

Веб-краулер (Freelance.ru, Habr Freelance)

Статистика и CSV-экспорт

JWT-авторизация, защита от SQL-инъекций
