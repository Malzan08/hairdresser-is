SET client_encoding = 'UTF8';
DROP VIEW IF EXISTS view_client_stats CASCADE;
DROP VIEW IF EXISTS view_branch_report CASCADE;
DROP TABLE IF EXISTS documents CASCADE;
DROP TABLE IF EXISTS price_history CASCADE;
DROP TABLE IF EXISTS haircuts CASCADE;
DROP TABLE IF EXISTS clients CASCADE;
DROP TABLE IF EXISTS employees CASCADE;
DROP TABLE IF EXISTS branches CASCADE;
DROP TABLE IF EXISTS user_roles CASCADE;
DROP TABLE IF EXISTS users CASCADE;
DROP TABLE IF EXISTS roles CASCADE;

CREATE TABLE roles (
    id_role SERIAL PRIMARY KEY,
    role_name VARCHAR(50) NOT NULL UNIQUE
);

CREATE TABLE users (
    id_user SERIAL PRIMARY KEY,
    login VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    CONSTRAINT chk_password_strength CHECK (
        char_length(password_hash) >= 8 AND
        password_hash ~ '[0-9]' AND
        password_hash ~ '[A-ZА-Я]' AND
        password_hash ~ '[^a-zA-Z0-9а-яА-Я\s]'
    )
);

CREATE TABLE user_roles (
    id_user INT NOT NULL REFERENCES users(id_user) ON DELETE CASCADE,
    id_role INT NOT NULL REFERENCES roles(id_role) ON DELETE CASCADE,
    PRIMARY KEY (id_user, id_role)
);

CREATE TABLE branches (
    id_branch SERIAL PRIMARY KEY,
    branch_name VARCHAR(100) NOT NULL,
    address VARCHAR(255) NOT NULL,
    phone VARCHAR(20) NOT NULL
);

CREATE TABLE employees (
    id_employee SERIAL PRIMARY KEY,
    id_user INT UNIQUE REFERENCES users(id_user) ON DELETE SET NULL,
    id_branch INT NOT NULL REFERENCES branches(id_branch) ON DELETE RESTRICT,
    last_name VARCHAR(50) NOT NULL,
    first_name VARCHAR(50) NOT NULL,
    middle_name VARCHAR(50),
    phone VARCHAR(20) NOT NULL,
    position VARCHAR(50) NOT NULL DEFAULT 'Мастер-парикмахер'
);

CREATE TABLE clients (
    id_client SERIAL PRIMARY KEY,
    id_user INT UNIQUE REFERENCES users(id_user) ON DELETE SET NULL,
    last_name VARCHAR(50) NOT NULL,
    first_name VARCHAR(50) NOT NULL,
    middle_name VARCHAR(50),
    phone VARCHAR(20) NOT NULL UNIQUE
);

CREATE TABLE haircuts (
    id_haircut SERIAL PRIMARY KEY,
    haircut_name VARCHAR(100) NOT NULL,
    gender VARCHAR(20) NOT NULL CHECK (gender IN ('Мужская', 'Женская', 'Унисекс'))
);

CREATE TABLE price_history (
    id_price_history SERIAL PRIMARY KEY,
    id_haircut INT NOT NULL REFERENCES haircuts(id_haircut) ON DELETE CASCADE,
    price NUMERIC(10, 2) NOT NULL CHECK (price > 0),
    change_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE documents (
    id_document SERIAL PRIMARY KEY,
    id_branch INT NOT NULL REFERENCES branches(id_branch) ON DELETE RESTRICT,
    id_employee INT NOT NULL REFERENCES employees(id_employee) ON DELETE RESTRICT,
    id_client INT NOT NULL REFERENCES clients(id_client) ON DELETE RESTRICT,
    id_haircut INT NOT NULL REFERENCES haircuts(id_haircut) ON DELETE RESTRICT,
    service_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    discount_rate NUMERIC(4, 2) NOT NULL DEFAULT 0.00,
    final_cost NUMERIC(10, 2) NOT NULL
);

CREATE OR REPLACE FUNCTION fn_process_haircut_document()
RETURNS TRIGGER AS $$
DECLARE
    v_base_price NUMERIC(10, 2);
    v_visit_count INT;
BEGIN
    SELECT price INTO v_base_price
    FROM price_history
    WHERE id_haircut = NEW.id_haircut AND change_date <= NEW.service_date
    ORDER BY change_date DESC
    LIMIT 1;

    IF v_base_price IS NULL THEN
        SELECT price INTO v_base_price FROM price_history
        WHERE id_haircut = NEW.id_haircut ORDER BY change_date DESC LIMIT 1;
    END IF;

    IF v_base_price IS NULL THEN
        RAISE EXCEPTION 'Для выбранной стрижки не установлена стоимость в истории цен!';
    END IF;

    SELECT COUNT(*) INTO v_visit_count
    FROM documents
    WHERE id_client = NEW.id_client;

    IF v_visit_count >= 4 THEN
        NEW.discount_rate := 3.00;
    ELSE
        NEW.discount_rate := 0.00;
    END IF;

    NEW.final_cost := ROUND(v_base_price * (1.0 - (NEW.discount_rate / 100.0)), 2);

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_calculate_haircut_cost
BEFORE INSERT ON documents
FOR EACH ROW
EXECUTE FUNCTION fn_process_haircut_document();

CREATE OR REPLACE VIEW view_client_stats AS
SELECT 
    c.id_client,
    CONCAT(c.last_name, ' ', c.first_name, ' ', COALESCE(c.middle_name, '')) AS client_name,
    c.phone,
    COUNT(d.id_document) AS total_visits,
    CASE WHEN COUNT(d.id_document) >= 5 THEN 'Постоянный (3% скидка)' ELSE 'Обычный' END AS client_status,
    COALESCE(SUM(d.final_cost), 0) AS total_spent
FROM clients c
LEFT JOIN documents d ON c.id_client = d.id_client
GROUP BY c.id_client, c.last_name, c.first_name, c.middle_name, c.phone;

CREATE OR REPLACE VIEW view_branch_report AS
SELECT 
    b.id_branch,
    b.branch_name,
    COUNT(d.id_document) AS services_rendered,
    COALESCE(SUM(d.final_cost), 0) AS total_revenue
FROM branches b
LEFT JOIN documents d ON b.id_branch = d.id_branch
GROUP BY b.id_branch, b.branch_name;

INSERT INTO roles (role_name) VALUES ('Admin'), ('Master'), ('Client');

INSERT INTO users (login, password_hash) VALUES
('admin', 'Admin!123'),
('master_ivan', 'Master!123'),
('client_elena', 'Client!123');

INSERT INTO user_roles (id_user, id_role) VALUES
(1, 1), -- admin -> Admin
(2, 2), -- master_ivan -> Master
(3, 3); -- client_elena -> Client

INSERT INTO branches (branch_name, address, phone) VALUES
('Филиал «Центральный»', 'ул. Тверская, д. 12', '+7(495)111-22-33'),
('Филиал «Северный»', 'Ленинградский пр-т, д. 45', '+7(495)222-33-44');

INSERT INTO employees (id_user, id_branch, last_name, first_name, middle_name, phone, position) VALUES
(2, 1, 'Иванов', 'Иван', 'Сергеевич', '+7(999)123-45-67', 'Топ-стилист');

INSERT INTO clients (id_user, last_name, first_name, middle_name, phone) VALUES
(3, 'Смирнова', 'Елена', 'Павловна', '+7(905)987-65-43'),
(NULL, 'Кузнецов', 'Алексей', 'Игоревич', '+7(916)555-44-33');

INSERT INTO haircuts (haircut_name, gender) VALUES
('Классическая мужская', 'Мужская'),
('Каре', 'Женская'),
('Пикси', 'Женская'),
('Фейд', 'Мужская');

INSERT INTO price_history (id_haircut, price, change_date) VALUES
(1, 1200.00, '2025-01-01 10:00:00'),
(2, 2000.00, '2025-01-01 10:00:00'),
(3, 2500.00, '2025-01-01 10:00:00'),
(4, 1500.00, '2025-01-01 10:00:00');

INSERT INTO documents (id_branch, id_employee, id_client, id_haircut, service_date, final_cost)
VALUES 
(1, 1, 1, 2, '2025-01-10 12:00:00', 0),
(1, 1, 1, 2, '2025-02-10 12:00:00', 0),
(1, 1, 1, 2, '2025-03-10 12:00:00', 0),
(1, 1, 1, 2, '2025-04-10 12:00:00', 0);