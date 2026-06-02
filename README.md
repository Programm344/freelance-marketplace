# Freelance Marketplace

Маркетплейс фрилансеров с веб-краулером

**Стек:** C++ Drogon + React TypeScript + PostgreSQL

---

## Быстрый запуск (Docker)

```bash
git clone https://github.com/Programm344/freelance-marketplace.git
cd freelance-marketplace
docker-compose up
Открыть: http://localhost:8080

Все включено: PostgreSQL, бэкенд, фронтенд, тестовые данные.
Аккаунты для входа
Роль	Логин	Пароль
Админ	admin@freelance.ru	123456
Модератор	moder@freelance.ru	123456
Фрилансер	ivan@freelance.ru	123456
Заказчик	company@freelance.ru	123456
Как проверить функционал
Шаг	Действие	Аккаунт
1	Зарегистрироваться	Новый пользователь
2	Заполнить профиль фрилансера	ivan@freelance.ru
3	Создать заказ	company@freelance.ru
4	Одобрить заказ (модерация)	moder@freelance.ru
5	Найти заказ и откликнуться	ivan@freelance.ru
6	Принять отклик	company@freelance.ru
7	Чат между участниками	Оба аккаунта
8	Завершить заказ и оставить отзыв	Оба аккаунта
9	Запустить краулер	admin@freelance.ru
10	Посмотреть статистику и CSV	admin@freelance.ru
API
Метод	Путь	Описание
POST	/api/auth/register	Регистрация
POST	/api/auth/login	Вход (JWT)
GET	/api/orders	Список заказов
POST	/api/orders	Создать заказ
GET	/api/orders/search	Поиск заказов
POST	/api/responses	Откликнуться
POST	/api/responses/{id}/accept	Принять отклик
GET	/api/moderation/orders	Модерация
POST	/api/crawler/start	Запуск краулера
GET	/api/admin/stats	Статистика
GET	/api/admin/export/csv	CSV экспорт
Функционал
4 роли с разграничением прав

CRUD заказов с 9 статусами

Отклики, чат, отзывы, рейтинги

Модерация заказов и заявок на роль

Веб-краулер (Freelance.ru, Habr Freelance)

Сохраненные поиски и уведомления

Статистика и CSV-экспорт

JWT-авторизация, защита от SQL-инъекций
