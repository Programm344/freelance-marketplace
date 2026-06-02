FROM ubuntu:22.04

RUN apt update && apt install -y \
    libpq-dev libjsoncpp25 libssl3 zlib1g libcurl4 \
    postgresql-client nodejs npm \
    && rm -rf /var/lib/apt/lists/*

COPY backend/build/freelance_backend /app/freelance_backend
COPY backend/config /app/config
COPY frontend /app/frontend
COPY database/migrations /app/migrations

WORKDIR /app/frontend
RUN npm install && npm run build
RUN mkdir -p /app/public && cp -r build/* /app/public/

WORKDIR /app
EXPOSE 8080

CMD ["/app/freelance_backend"]
