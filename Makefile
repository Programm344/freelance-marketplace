.PHONY: install db build run clean all

all: install db build run

install:
	@echo "=== Установка зависимостей ==="
	sudo apt update
	sudo apt install -y postgresql postgresql-client cmake g++ libpq-dev libcurl4-openssl-dev libssl-dev nodejs npm
	sudo service postgresql start
	cd frontend && npm install

db:
	@echo "=== Создание базы данных ==="
	sudo -u postgres psql -c "CREATE USER freelancer WITH PASSWORD 'freelancer_pass';" 2>/dev/null || true
	sudo -u postgres psql -c "CREATE DATABASE freelance_marketplace OWNER freelancer;" 2>/dev/null || true
	sudo -u postgres psql -d freelance_marketplace -c "GRANT ALL ON SCHEMA public TO freelancer;" 2>/dev/null || true
	PGPASSWORD=freelancer_pass psql -h 127.0.0.1 -U freelancer -d freelance_marketplace -f database/migrations/001_initial_schema.sql
	PGPASSWORD=freelancer_pass psql -h 127.0.0.1 -U freelancer -d freelance_marketplace -f database/migrations/002_seed_data.sql

build:
	@echo "=== Сборка бэкенда ==="
	cd backend && mkdir -p build && cd build && cmake .. && make -j$$(nproc)

run:
	@echo "=== Запуск ==="
	cd backend && ./build/freelance_backend &
	cd frontend && npm start

clean:
	@echo "=== Очистка ==="
	killall freelance_backend 2>/dev/null || true
	rm -rf backend/build
