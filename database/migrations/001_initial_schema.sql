-- Роли пользователей
CREATE TYPE user_role AS ENUM ('freelancer', 'customer', 'moderator', 'admin');

-- Статусы заказов
CREATE TYPE order_status AS ENUM ('draft', 'on_moderation', 'published', 'in_progress', 'completed', 'cancelled', 'rejected');

-- Статусы откликов
CREATE TYPE response_status AS ENUM ('sent', 'viewed', 'accepted', 'rejected', 'withdrawn');

-- Статусы внешних заказов
CREATE TYPE external_order_status AS ENUM ('new', 'actual', 'archived', 'error');

-- Статусы источников краулера
CREATE TYPE source_status AS ENUM ('active', 'disabled', 'error');

-- Пользователи
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    role user_role DEFAULT 'freelancer',
    is_blocked BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Профили фрилансеров
CREATE TABLE freelancer_profiles (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
    display_name VARCHAR(255),
    specialization VARCHAR(255),
    experience_description TEXT,
    skills TEXT[],
    portfolio JSONB DEFAULT '[]',
    hourly_rate DECIMAL(10,2),
    contact_info JSONB DEFAULT '{}',
    rating DECIMAL(3,2) DEFAULT 0.0,
    is_available BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Профили заказчиков
CREATE TABLE customer_profiles (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
    company_name VARCHAR(255),
    description TEXT,
    contact_info JSONB DEFAULT '{}',
    rating DECIMAL(3,2) DEFAULT 0.0,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Категории заказов
CREATE TABLE categories (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    slug VARCHAR(255) UNIQUE NOT NULL,
    parent_id INTEGER REFERENCES categories(id),
    created_at TIMESTAMP DEFAULT NOW()
);

-- Заказы
CREATE TABLE orders (
    id SERIAL PRIMARY KEY,
    customer_id INTEGER REFERENCES users(id),
    title VARCHAR(500) NOT NULL,
    description TEXT,
    category_id INTEGER REFERENCES categories(id),
    required_skills TEXT[],
    budget DECIMAL(12,2),
    deadline DATE,
    payment_format VARCHAR(100),
    attachments JSONB DEFAULT '[]',
    status order_status DEFAULT 'draft',
    source VARCHAR(50) DEFAULT 'internal', -- internal или external
    external_url TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- История статусов заказов
CREATE TABLE order_status_history (
    id SERIAL PRIMARY KEY,
    order_id INTEGER REFERENCES orders(id) ON DELETE CASCADE,
    old_status order_status,
    new_status order_status,
    changed_by INTEGER REFERENCES users(id),
    comment TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Отклики на заказы
CREATE TABLE responses (
    id SERIAL PRIMARY KEY,
    order_id INTEGER REFERENCES orders(id) ON DELETE CASCADE,
    freelancer_id INTEGER REFERENCES users(id),
    message TEXT,
    proposed_budget DECIMAL(12,2),
    proposed_deadline DATE,
    status response_status DEFAULT 'sent',
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Сохранённые поиски
CREATE TABLE saved_searches (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(255),
    categories INTEGER[],
    keywords TEXT[],
    skills TEXT[],
    budget_min DECIMAL(12,2),
    budget_max DECIMAL(12,2),
    source VARCHAR(50),
    created_at TIMESTAMP DEFAULT NOW()
);

-- Внешние заказы (с краулера)
CREATE TABLE external_orders (
    id SERIAL PRIMARY KEY,
    title VARCHAR(500),
    description TEXT,
    category VARCHAR(255),
    skills TEXT[],
    budget DECIMAL(12,2),
    deadline VARCHAR(255),
    source_url TEXT,
    source_name VARCHAR(255),
    status external_order_status DEFAULT 'new',
    discovered_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Источники для краулера
CREATE TABLE crawler_sources (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    base_url TEXT NOT NULL,
    crawl_rules JSONB DEFAULT '{}',
    extract_rules JSONB DEFAULT '{}',
    frequency INTEGER DEFAULT 3600, -- в секундах
    status source_status DEFAULT 'active',
    last_run_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Логи краулера
CREATE TABLE crawl_logs (
    id SERIAL PRIMARY KEY,
    source_id INTEGER REFERENCES crawler_sources(id),
    status VARCHAR(50),
    items_found INTEGER DEFAULT 0,
    items_new INTEGER DEFAULT 0,
    error_message TEXT,
    started_at TIMESTAMP,
    finished_at TIMESTAMP
);

-- Отзывы
CREATE TABLE reviews (
    id SERIAL PRIMARY KEY,
    author_id INTEGER REFERENCES users(id),
    target_id INTEGER REFERENCES users(id),
    order_id INTEGER REFERENCES orders(id),
    rating INTEGER CHECK (rating >= 1 AND rating <= 5),
    comment TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Уведомления
CREATE TABLE notifications (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
    type VARCHAR(100),
    title VARCHAR(500),
    message TEXT,
    data JSONB DEFAULT '{}',
    is_read BOOLEAN DEFAULT FALSE,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Сообщения в заказе
CREATE TABLE messages (
    id SERIAL PRIMARY KEY,
    order_id INTEGER REFERENCES orders(id) ON DELETE CASCADE,
    sender_id INTEGER REFERENCES users(id),
    message TEXT,
    attachments JSONB DEFAULT '[]',
    created_at TIMESTAMP DEFAULT NOW()
);

-- Жалобы
CREATE TABLE complaints (
    id SERIAL PRIMARY KEY,
    author_id INTEGER REFERENCES users(id),
    target_type VARCHAR(50), -- 'order', 'user', 'external_order'
    target_id INTEGER,
    reason TEXT,
    status VARCHAR(50) DEFAULT 'pending', -- pending, reviewed, resolved
    moderator_id INTEGER REFERENCES users(id),
    resolution TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Индексы для быстрого поиска
CREATE INDEX idx_orders_status ON orders(status);
CREATE INDEX idx_orders_category ON orders(category_id);
CREATE INDEX idx_orders_budget ON orders(budget);
CREATE INDEX idx_orders_deadline ON orders(deadline);
CREATE INDEX idx_responses_order ON responses(order_id);
CREATE INDEX idx_responses_freelancer ON responses(freelancer_id);
CREATE INDEX idx_notifications_user ON notifications(user_id);
CREATE INDEX idx_external_orders_status ON external_orders(status);
CREATE INDEX idx_external_orders_source ON external_orders(source_name);

-- Заявки на смену роли
CREATE TABLE IF NOT EXISTS role_requests (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id),
    requested_role VARCHAR(50) NOT NULL,
    status VARCHAR(50) DEFAULT 'pending',
    company_name VARCHAR(255),
    description TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Заявки на смену роли
CREATE TABLE IF NOT EXISTS role_requests (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id),
    requested_role VARCHAR(50) NOT NULL,
    status VARCHAR(50) DEFAULT 'pending',
    company_name VARCHAR(255),
    description TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);
