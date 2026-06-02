INSERT INTO users (email, password_hash, role) VALUES
('admin@freelance.ru', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'admin'),
('moder@freelance.ru', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'moderator'),
('ivan@freelance.ru', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'freelancer'),
('company@freelance.ru', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', 'customer');

INSERT INTO freelancer_profiles (user_id, display_name, specialization, experience_description, skills, hourly_rate, rating, is_available) VALUES
(3, 'Иван Петров', 'Frontend Developer', '5 лет опыта', '{React,TypeScript}', 2000, 5.0, true);

INSERT INTO customer_profiles (user_id, company_name, description, rating) VALUES
(4, 'ООО ТехноПроект', 'Разработка ПО', 4.5);

INSERT INTO categories (id, name, slug) VALUES
(1, 'Web Development', 'web-dev'),
(2, 'Mobile Development', 'mobile-dev');

INSERT INTO orders (customer_id, title, description, category_id, required_skills, budget, deadline, status) VALUES
(4, 'Разработка сайта на React', 'Нужен современный сайт', 1, '{React,TypeScript}', 500000, '2025-07-15', 'published'),
(4, 'Бэкенд на Python', 'REST API', 1, '{Python,Django}', 350000, '2025-08-01', 'published');

INSERT INTO crawler_sources (name, base_url, status) VALUES
('Habr Freelance', 'https://freelance.habr.com/tasks', 'active');
