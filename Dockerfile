FROM ubuntu:22.04
RUN apt update && apt install -y libpq-dev libjsoncpp25 libssl3 zlib1g libcurl4 && rm -rf /var/lib/apt/lists/*
COPY backend/build/freelance_backend /app/freelance_backend
COPY backend/config /app/config
COPY backend/public /app/public
WORKDIR /app
EXPOSE 8080
CMD ["./freelance_backend"]
