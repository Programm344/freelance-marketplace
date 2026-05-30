# Freelance Marketplace

Маркетплейс фрилансеров с веб-краулером. Стек: C++ Drogon + React + PostgreSQL.

---

## Как запустить (WSL / Ubuntu 22.04+)

### Шаг 1. Клонировать репозиторий
```bash
git clone https://github.com/Programm344/freelance-marketplace.git  # Скачать проект
cd freelance-marketplace                                            # Зайти в папку
sudo apt update                                                      # Обновить пакеты
sudo apt install -y postgresql postgresql-client                     # Установить PostgreSQL
sudo service postgresql start                                        # Запустить
# Создать пользователя БД
sudo -u postgres psql -c "CREATE USER freelancer WITH PASSWORD 'freelancer_pass';"

# Создать базу данных
sudo -u postgres psql -c "CREATE DATABASE freelance_marketplace OWNER freelancer;"

# Дать права
sudo -u postgres psql -d freelance_marketplace -c "GRANT ALL ON SCHEMA public TO freelancer;"

# Создать таблицы
PGPASSWORD=freelancer_pass psql -h 127.0.0.1 -U freelancer -d freelance_marketplace -f database/migrations/001_initial_schema.sql

# Наполнить тестовыми данными (пользователи, заказы, отклики)
PGPASSWORD=freelancer_pass psql -h 127.0.0.1 -U freelancer -d freelance_marketplace -f database/migrations/002_seed_data.sql
cd backend                                   # Зайти в папку бэкенда
chmod +x build/freelance_backend             # Сделать файл исполняемым
./build/freelance_backend                    # Запустить сервер
# Сервер работает на http://localhost:8080
cd frontend               # Зайти в папку фронтенда
npm install               # Установить зависимости (один раз)
npm start                 # Запустить интерфейс
# Открыть http://localhost:3000
#Шаг 6 войти в систему 
Роль	Логин	Пароль
Админ	admin@freelance.ru	123456
Модератор	moder@freelance.ru	123456
Фрилансер	ivan@freelance.ru	123456
Заказчик	company@freelance.ru	123456
freelance_marketplace/
├── backend/
│   ├── controllers/       # 15 контроллеров API
│   ├── crawler/           # Веб-краулер
│   ├── middleware/        # JWT авторизация
│   ├── models/            # Модели данных
│   ├── services/          # Бизнес-логика
│   ├── utils/             # Хелперы и JWT
│   ├── config/            # Конфигурация
│   └── build/             # Скомпилированный бинарник
├── frontend/
│   └── src/
│       ├── pages/         # 15 страниц
│       ├── components/    # UI компоненты
│       └── services/      # API клиент
└── database/
    └── migrations/        # SQL миграции (15 таблиц + тестовые данные)
API (основные эндпоинты)
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
4 роли: фрилансер, заказчик, модератор, админ

CRUD заказов с 9 статусами

Отклики, чат, отзывы, рейтинги

Модерация заказов и заявок на роль

Веб-краулер (Freelance.ru, Habr Freelance)

Статистика и CSV-экспорт

JWT-авторизация

Защита от SQL-инъекций
