#include "DBManager.h"

DBManager& DBManager::instance() {
    static DBManager instance;
    return instance;
}

DBManager::DBManager(QObject *parent) : QObject(parent) {
    if (QSqlDatabase::contains("hairdresser_connection")) {
        m_db = QSqlDatabase::database("hairdresser_connection");
    } else {
        m_db = QSqlDatabase::addDatabase("QPSQL", "hairdresser_connection");
    }
}

DBManager::~DBManager() {
    disconnectDatabase();
}

bool DBManager::connectDatabase(const QString &host, int port, const QString &dbName, const QString &user, const QString &password) {
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

void DBManager::disconnectDatabase() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DBManager::isOpen() const {
    return m_db.isOpen();
}

QString DBManager::lastError() const {
    return m_lastError;
}

std::optional<User> DBManager::authenticateUser(const QString &login, const QString &password) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id_user, login, password_hash FROM users WHERE login = :login AND password_hash = :password");
    query.bindValue(":login", login);
    query.bindValue(":password", password);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return std::nullopt;
    }

    if (query.next()) {
        User user;
        user.idUser = query.value("id_user").toInt();
        user.login = query.value("login").toString();
        user.passwordHash = query.value("password_hash").toString();
        user.roles = getUserRoles(user.idUser);
        return user;
    }

    m_lastError = "Пользователь с такими данными не найден.";
    return std::nullopt;
}

QVector<Role> DBManager::getUserRoles(int userId) {
    QVector<Role> roles;
    QSqlQuery query(m_db);
    query.prepare("SELECT r.id_role, r.role_name FROM roles r "
                  "JOIN user_roles ur ON r.id_role = ur.id_role "
                  "WHERE ur.id_user = :userId");
    query.bindValue(":userId", userId);

    if (query.exec()) {
        while (query.next()) {
            Role role;
            role.idRole = query.value("id_role").toInt();
            role.roleName = query.value("role_name").toString();
            roles.append(role);
        }
    } else {
        m_lastError = query.lastError().text();
    }

    return roles;
}

std::optional<User> DBManager::getUserByLogin(const QString &login) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id_user, login, password_hash FROM users WHERE login = :login");
    query.bindValue(":login", login);

    if (query.exec() && query.next()) {
        User user;
        user.idUser = query.value("id_user").toInt();
        user.login = query.value("login").toString();
        user.passwordHash = query.value("password_hash").toString();
        user.roles = getUserRoles(user.idUser);
        return user;
    }
    return std::nullopt;
}

std::optional<User> DBManager::getUserById(int userId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id_user, login, password_hash FROM users WHERE id_user = :userId");
    query.bindValue(":userId", userId);

    if (query.exec() && query.next()) {
        User user;
        user.idUser = query.value("id_user").toInt();
        user.login = query.value("login").toString();
        user.passwordHash = query.value("password_hash").toString();
        user.roles = getUserRoles(user.idUser);
        return user;
    }
    return std::nullopt;
}

bool DBManager::checkLoginExists(const QString &login) {
    QSqlQuery query(m_db);
    query.prepare("SELECT 1 FROM users WHERE login = :login");
    query.bindValue(":login", login);
    if (query.exec()) {
        return query.next();
    }
    m_lastError = query.lastError().text();
    return false;
}

bool DBManager::registerUser(const QString &login, const QString &password, const QString &roleName, int *outUserId) {
    if (checkLoginExists(login)) {
        m_lastError = "Пользователь с таким логином уже существует.";
        return false;
    }

    QSqlQuery roleQuery(m_db);
    roleQuery.prepare("SELECT id_role FROM roles WHERE role_name = :roleName");
    roleQuery.bindValue(":roleName", roleName);
    if (!roleQuery.exec() || !roleQuery.next()) {
        m_lastError = "Указанная роль не найдена в системе.";
        return false;
    }
    int roleId = roleQuery.value("id_role").toInt();

    QSqlQuery userQuery(m_db);
    userQuery.prepare("INSERT INTO users (login, password_hash) VALUES (:login, :password) RETURNING id_user");
    userQuery.bindValue(":login", login);
    userQuery.bindValue(":password", password);

    if (!userQuery.exec() || !userQuery.next()) {
        m_lastError = userQuery.lastError().text();
        return false;
    }

    int newUserId = userQuery.value(0).toInt();

    QSqlQuery mapQuery(m_db);
    mapQuery.prepare("INSERT INTO user_roles (id_user, id_role) VALUES (:userId, :roleId)");
    mapQuery.bindValue(":userId", newUserId);
    mapQuery.bindValue(":roleId", roleId);

    if (!mapQuery.exec()) {
        m_lastError = mapQuery.lastError().text();
        return false;
    }

    if (outUserId) {
        *outUserId = newUserId;
    }
    return true;
}

QVector<Role> DBManager::getAllRoles() {
    QVector<Role> roles;
    QSqlQuery query(m_db);
    if (query.exec("SELECT id_role, role_name FROM roles ORDER BY id_role ASC")) {
        while (query.next()) {
            Role r;
            r.idRole = query.value("id_role").toInt();
            r.roleName = query.value("role_name").toString();
            roles.append(r);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return roles;
}

bool DBManager::updateUserPassword(int userId, const QString &newPassword) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET password_hash = :newPassword WHERE id_user = :userId");
    query.bindValue(":newPassword", newPassword);
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QVector<User> DBManager::getAllUsers() {
    QVector<User> users;
    QSqlQuery query(m_db);
    if (query.exec("SELECT id_user, login, password_hash FROM users ORDER BY id_user ASC")) {
        while (query.next()) {
            User u;
            u.idUser = query.value("id_user").toInt();
            u.login = query.value("login").toString();
            u.passwordHash = query.value("password_hash").toString();
            u.roles = getUserRoles(u.idUser);
            users.append(u);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return users;
}

bool DBManager::deleteUser(int userId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM users WHERE id_user = :userId");
    query.bindValue(":userId", userId);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::setUserRole(int userId, int roleId) {
    QSqlQuery delQuery(m_db);
    delQuery.prepare("DELETE FROM user_roles WHERE id_user = :userId");
    delQuery.bindValue(":userId", userId);
    if (!delQuery.exec()) {
        m_lastError = delQuery.lastError().text();
        return false;
    }

    QSqlQuery insQuery(m_db);
    insQuery.prepare("INSERT INTO user_roles (id_user, id_role) VALUES (:userId, :roleId)");
    insQuery.bindValue(":userId", userId);
    insQuery.bindValue(":roleId", roleId);
    if (!insQuery.exec()) {
        m_lastError = insQuery.lastError().text();
        return false;
    }
    return true;
}

QVector<Branch> DBManager::getAllBranches() {
    QVector<Branch> branches;
    QSqlQuery query(m_db);
    if (query.exec("SELECT id_branch, branch_name, address, phone FROM branches ORDER BY id_branch ASC")) {
        while (query.next()) {
            Branch b;
            b.idBranch = query.value("id_branch").toInt();
            b.branchName = query.value("branch_name").toString();
            b.address = query.value("address").toString();
            b.phone = query.value("phone").toString();
            branches.append(b);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return branches;
}

std::optional<Branch> DBManager::getBranchById(int branchId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id_branch, branch_name, address, phone FROM branches WHERE id_branch = :branchId");
    query.bindValue(":branchId", branchId);
    if (query.exec() && query.next()) {
        Branch b;
        b.idBranch = query.value("id_branch").toInt();
        b.branchName = query.value("branch_name").toString();
        b.address = query.value("address").toString();
        b.phone = query.value("phone").toString();
        return b;
    }
    return std::nullopt;
}

bool DBManager::addBranch(const QString &name, const QString &address, const QString &phone, int *outBranchId) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO branches (branch_name, address, phone) VALUES (:name, :address, :phone) RETURNING id_branch");
    query.bindValue(":name", name);
    query.bindValue(":address", address);
    query.bindValue(":phone", phone);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return false;
    }
    if (outBranchId) {
        *outBranchId = query.value(0).toInt();
    }
    return true;
}

bool DBManager::updateBranch(int branchId, const QString &name, const QString &address, const QString &phone) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE branches SET branch_name = :name, address = :address, phone = :phone WHERE id_branch = :branchId");
    query.bindValue(":name", name);
    query.bindValue(":address", address);
    query.bindValue(":phone", phone);
    query.bindValue(":branchId", branchId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::deleteBranch(int branchId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM branches WHERE id_branch = :branchId");
    query.bindValue(":branchId", branchId);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QVector<Employee> DBManager::getAllEmployees() {
    QVector<Employee> employees;
    QSqlQuery query(m_db);
    if (query.exec("SELECT e.id_employee, e.id_user, e.id_branch, e.last_name, e.first_name, e.middle_name, "
                   "e.phone, e.position, b.branch_name "
                   "FROM employees e "
                   "JOIN branches b ON e.id_branch = b.id_branch "
                   "ORDER BY e.id_employee ASC")) {
        while (query.next()) {
            Employee emp;
            emp.idEmployee = query.value("id_employee").toInt();
            emp.idUser = query.value("id_user").toInt();
            emp.idBranch = query.value("id_branch").toInt();
            emp.lastName = query.value("last_name").toString();
            emp.firstName = query.value("first_name").toString();
            emp.middleName = query.value("middle_name").toString();
            emp.phone = query.value("phone").toString();
            emp.position = query.value("position").toString();
            emp.branchName = query.value("branch_name").toString();
            employees.append(emp);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return employees;
}

QVector<Employee> DBManager::getEmployeesByBranch(int branchId) {
    QVector<Employee> employees;
    QSqlQuery query(m_db);
    query.prepare("SELECT e.id_employee, e.id_user, e.id_branch, e.last_name, e.first_name, e.middle_name, "
                  "e.phone, e.position, b.branch_name "
                  "FROM employees e "
                  "JOIN branches b ON e.id_branch = b.id_branch "
                  "WHERE e.id_branch = :branchId ORDER BY e.id_employee ASC");
    query.bindValue(":branchId", branchId);

    if (query.exec()) {
        while (query.next()) {
            Employee emp;
            emp.idEmployee = query.value("id_employee").toInt();
            emp.idUser = query.value("id_user").toInt();
            emp.idBranch = query.value("id_branch").toInt();
            emp.lastName = query.value("last_name").toString();
            emp.firstName = query.value("first_name").toString();
            emp.middleName = query.value("middle_name").toString();
            emp.phone = query.value("phone").toString();
            emp.position = query.value("position").toString();
            emp.branchName = query.value("branch_name").toString();
            employees.append(emp);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return employees;
}

std::optional<Employee> DBManager::getEmployeeByUserId(int userId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT e.id_employee, e.id_user, e.id_branch, e.last_name, e.first_name, e.middle_name, "
                  "e.phone, e.position, b.branch_name "
                  "FROM employees e "
                  "JOIN branches b ON e.id_branch = b.id_branch "
                  "WHERE e.id_user = :userId");
    query.bindValue(":userId", userId);

    if (query.exec() && query.next()) {
        Employee emp;
        emp.idEmployee = query.value("id_employee").toInt();
        emp.idUser = query.value("id_user").toInt();
        emp.idBranch = query.value("id_branch").toInt();
        emp.lastName = query.value("last_name").toString();
        emp.firstName = query.value("first_name").toString();
        emp.middleName = query.value("middle_name").toString();
        emp.phone = query.value("phone").toString();
        emp.position = query.value("position").toString();
        emp.branchName = query.value("branch_name").toString();
        return emp;
    }
    return std::nullopt;
}

std::optional<Employee> DBManager::getEmployeeById(int employeeId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT e.id_employee, e.id_user, e.id_branch, e.last_name, e.first_name, e.middle_name, "
                  "e.phone, e.position, b.branch_name "
                  "FROM employees e "
                  "JOIN branches b ON e.id_branch = b.id_branch "
                  "WHERE e.id_employee = :employeeId");
    query.bindValue(":employeeId", employeeId);

    if (query.exec() && query.next()) {
        Employee emp;
        emp.idEmployee = query.value("id_employee").toInt();
        emp.idUser = query.value("id_user").toInt();
        emp.idBranch = query.value("id_branch").toInt();
        emp.lastName = query.value("last_name").toString();
        emp.firstName = query.value("first_name").toString();
        emp.middleName = query.value("middle_name").toString();
        emp.phone = query.value("phone").toString();
        emp.position = query.value("position").toString();
        emp.branchName = query.value("branch_name").toString();
        return emp;
    }
    return std::nullopt;
}

QVector<Haircut> DBManager::getAllHaircuts() {
    QVector<Haircut> haircuts;
    QSqlQuery query(m_db);
    QString qStr = "SELECT h.id_haircut, h.haircut_name, h.gender, "
                   "COALESCE((SELECT ph.price FROM price_history ph "
                   "WHERE ph.id_haircut = h.id_haircut "
                   "ORDER BY ph.change_date DESC, ph.id_price_history DESC LIMIT 1), 0.0) AS current_price "
                   "FROM haircuts h ORDER BY h.id_haircut ASC";
    if (query.exec(qStr)) {
        while (query.next()) {
            Haircut h;
            h.idHaircut = query.value("id_haircut").toInt();
            h.haircutName = query.value("haircut_name").toString();
            h.gender = query.value("gender").toString();
            h.currentPrice = query.value("current_price").toDouble();
            haircuts.append(h);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return haircuts;
}

std::optional<Haircut> DBManager::getHaircutById(int haircutId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT h.id_haircut, h.haircut_name, h.gender, "
                  "COALESCE((SELECT ph.price FROM price_history ph "
                  "WHERE ph.id_haircut = h.id_haircut "
                  "ORDER BY ph.change_date DESC, ph.id_price_history DESC LIMIT 1), 0.0) AS current_price "
                  "FROM haircuts h WHERE h.id_haircut = :haircutId");
    query.bindValue(":haircutId", haircutId);

    if (query.exec() && query.next()) {
        Haircut h;
        h.idHaircut = query.value("id_haircut").toInt();
        h.haircutName = query.value("haircut_name").toString();
        h.gender = query.value("gender").toString();
        h.currentPrice = query.value("current_price").toDouble();
        return h;
    }
    return std::nullopt;
}

bool DBManager::addHaircut(const QString &name, const QString &gender, double initialPrice, int *outHaircutId) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO haircuts (haircut_name, gender) VALUES (:name, :gender) RETURNING id_haircut");
    query.bindValue(":name", name);
    query.bindValue(":gender", gender);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return false;
    }

    int newId = query.value(0).toInt();

    if (initialPrice > 0.0) {
        QSqlQuery priceQuery(m_db);
        priceQuery.prepare("INSERT INTO price_history (id_haircut, price) VALUES (:haircutId, :price)");
        priceQuery.bindValue(":haircutId", newId);
        priceQuery.bindValue(":price", initialPrice);
        if (!priceQuery.exec()) {
            m_lastError = priceQuery.lastError().text();
            return false;
        }
    }

    if (outHaircutId) {
        *outHaircutId = newId;
    }
    return true;
}

bool DBManager::updateHaircut(int haircutId, const QString &name, const QString &gender) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE haircuts SET haircut_name = :name, gender = :gender WHERE id_haircut = :haircutId");
    query.bindValue(":name", name);
    query.bindValue(":gender", gender);
    query.bindValue(":haircutId", haircutId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::deleteHaircut(int haircutId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM haircuts WHERE id_haircut = :haircutId");
    query.bindValue(":haircutId", haircutId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::addPriceHistory(int haircutId, double newPrice) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO price_history (id_haircut, price, change_date) VALUES (:haircutId, :price, CURRENT_TIMESTAMP)");
    query.bindValue(":haircutId", haircutId);
    query.bindValue(":price", newPrice);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QVector<PriceHistory> DBManager::getPriceHistoryByHaircut(int haircutId) {
    QVector<PriceHistory> history;
    QSqlQuery query(m_db);
    query.prepare("SELECT id_price_history, id_haircut, price, change_date FROM price_history "
                  "WHERE id_haircut = :haircutId ORDER BY change_date DESC, id_price_history DESC");
    query.bindValue(":haircutId", haircutId);

    if (query.exec()) {
        while (query.next()) {
            PriceHistory ph;
            ph.idPriceHistory = query.value("id_price_history").toInt();
            ph.idHaircut = query.value("id_haircut").toInt();
            ph.price = query.value("price").toDouble();
            ph.changeDate = query.value("change_date").toDateTime();
            history.append(ph);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return history;
}

QVector<Client> DBManager::getAllClients() {
    QVector<Client> clients;
    QSqlQuery query(m_db);
    if (query.exec("SELECT id_client, id_user, last_name, first_name, middle_name, phone FROM clients ORDER BY id_client ASC")) {
        while (query.next()) {
            Client c;
            c.idClient = query.value("id_client").toInt();
            c.idUser = query.value("id_user").isNull() ? 0 : query.value("id_user").toInt();
            c.lastName = query.value("last_name").toString();
            c.firstName = query.value("first_name").toString();
            c.middleName = query.value("middle_name").toString();
            c.phone = query.value("phone").toString();
            clients.append(c);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return clients;
}

std::optional<Client> DBManager::getClientById(int clientId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id_client, id_user, last_name, first_name, middle_name, phone FROM clients WHERE id_client = :clientId");
    query.bindValue(":clientId", clientId);

    if (query.exec() && query.next()) {
        Client c;
        c.idClient = query.value("id_client").toInt();
        c.idUser = query.value("id_user").isNull() ? 0 : query.value("id_user").toInt();
        c.lastName = query.value("last_name").toString();
        c.firstName = query.value("first_name").toString();
        c.middleName = query.value("middle_name").toString();
        c.phone = query.value("phone").toString();
        return c;
    }
    return std::nullopt;
}

std::optional<Client> DBManager::getClientByPhone(const QString &phone) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id_client, id_user, last_name, first_name, middle_name, phone FROM clients WHERE phone = :phone");
    query.bindValue(":phone", phone);

    if (query.exec() && query.next()) {
        Client c;
        c.idClient = query.value("id_client").toInt();
        c.idUser = query.value("id_user").isNull() ? 0 : query.value("id_user").toInt();
        c.lastName = query.value("last_name").toString();
        c.firstName = query.value("first_name").toString();
        c.middleName = query.value("middle_name").toString();
        c.phone = query.value("phone").toString();
        return c;
    }
    return std::nullopt;
}

std::optional<Client> DBManager::getClientByUserId(int userId) {
    QSqlQuery query(m_db);
    query.prepare("SELECT id_client, id_user, last_name, first_name, middle_name, phone FROM clients WHERE id_user = :userId");
    query.bindValue(":userId", userId);

    if (query.exec() && query.next()) {
        Client c;
        c.idClient = query.value("id_client").toInt();
        c.idUser = query.value("id_user").isNull() ? 0 : query.value("id_user").toInt();
        c.lastName = query.value("last_name").toString();
        c.firstName = query.value("first_name").toString();
        c.middleName = query.value("middle_name").toString();
        c.phone = query.value("phone").toString();
        return c;
    }
    return std::nullopt;
}

bool DBManager::addClient(const QString &lastName, const QString &firstName, const QString &middleName, const QString &phone, std::optional<int> userId, int *outClientId) {
    QSqlQuery query(m_db);
    if (userId.has_value()) {
        query.prepare("INSERT INTO clients (id_user, last_name, first_name, middle_name, phone) "
                      "VALUES (:userId, :lastName, :firstName, :middleName, :phone) RETURNING id_client");
        query.bindValue(":userId", userId.value());
    } else {
        query.prepare("INSERT INTO clients (last_name, first_name, middle_name, phone) "
                      "VALUES (:lastName, :firstName, :middleName, :phone) RETURNING id_client");
    }
    query.bindValue(":lastName", lastName);
    query.bindValue(":firstName", firstName);
    query.bindValue(":middleName", middleName.isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : middleName);
    query.bindValue(":phone", phone);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return false;
    }

    if (outClientId) {
        *outClientId = query.value(0).toInt();
    }
    return true;
}

bool DBManager::updateClientPhone(int clientId, const QString &newPhone) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE clients SET phone = :phone WHERE id_client = :clientId");
    query.bindValue(":phone", newPhone);
    query.bindValue(":clientId", clientId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::updateClient(int clientId, const QString &lastName, const QString &firstName, const QString &middleName, const QString &phone) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE clients SET last_name = :lastName, first_name = :firstName, "
                  "middle_name = :middleName, phone = :phone WHERE id_client = :clientId");
    query.bindValue(":lastName", lastName);
    query.bindValue(":firstName", firstName);
    query.bindValue(":middleName", middleName.isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : middleName);
    query.bindValue(":phone", phone);
    query.bindValue(":clientId", clientId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::deleteClient(int clientId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM clients WHERE id_client = :clientId");
    query.bindValue(":clientId", clientId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::addDocument(int branchId, int employeeId, int clientId, int haircutId, const QDateTime &serviceDate, int *outDocId) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO documents (id_branch, id_employee, id_client, id_haircut, service_date, final_cost) "
                  "VALUES (:branchId, :employeeId, :clientId, :haircutId, :serviceDate, 0) RETURNING id_document");
    query.bindValue(":branchId", branchId);
    query.bindValue(":employeeId", employeeId);
    query.bindValue(":clientId", clientId);
    query.bindValue(":haircutId", haircutId);
    query.bindValue(":serviceDate", serviceDate);

    if (!query.exec() || !query.next()) {
        m_lastError = query.lastError().text();
        return false;
    }

    if (outDocId) {
        *outDocId = query.value(0).toInt();
    }
    return true;
}

QVector<Document> DBManager::getAllDocuments() {
    QVector<Document> documents;
    QSqlQuery query(m_db);
    QString qStr = "SELECT d.id_document, d.id_branch, d.id_employee, d.id_client, d.id_haircut, "
                   "d.service_date, d.discount_rate, d.final_cost, "
                   "b.branch_name, "
                   "CONCAT(e.last_name, ' ', SUBSTRING(e.first_name, 1, 1), '.', "
                   "       CASE WHEN e.middle_name IS NOT NULL AND LENGTH(e.middle_name) > 0 "
                   "            THEN CONCAT(SUBSTRING(e.middle_name, 1, 1), '.') ELSE '' END) AS employee_name, "
                   "CONCAT(c.last_name, ' ', c.first_name, ' ', COALESCE(c.middle_name, '')) AS client_name, "
                   "h.haircut_name "
                   "FROM documents d "
                   "JOIN branches b ON d.id_branch = b.id_branch "
                   "JOIN employees e ON d.id_employee = e.id_employee "
                   "JOIN clients c ON d.id_client = c.id_client "
                   "JOIN haircuts h ON d.id_haircut = h.id_haircut "
                   "ORDER BY d.service_date DESC, d.id_document DESC";

    if (query.exec(qStr)) {
        while (query.next()) {
            Document doc;
            doc.idDocument = query.value("id_document").toInt();
            doc.idBranch = query.value("id_branch").toInt();
            doc.idEmployee = query.value("id_employee").toInt();
            doc.idClient = query.value("id_client").toInt();
            doc.idHaircut = query.value("id_haircut").toInt();
            doc.serviceDate = query.value("service_date").toDateTime();
            doc.discountRate = query.value("discount_rate").toDouble();
            doc.finalCost = query.value("final_cost").toDouble();
            doc.branchName = query.value("branch_name").toString();
            doc.employeeName = query.value("employee_name").toString();
            doc.clientName = query.value("client_name").toString();
            doc.haircutName = query.value("haircut_name").toString();
            documents.append(doc);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return documents;
}

QVector<Document> DBManager::getDocumentsByBranch(int branchId) {
    QVector<Document> documents;
    QSqlQuery query(m_db);
    query.prepare("SELECT d.id_document, d.id_branch, d.id_employee, d.id_client, d.id_haircut, "
                  "d.service_date, d.discount_rate, d.final_cost, "
                  "b.branch_name, "
                  "CONCAT(e.last_name, ' ', SUBSTRING(e.first_name, 1, 1), '.', "
                  "       CASE WHEN e.middle_name IS NOT NULL AND LENGTH(e.middle_name) > 0 "
                  "            THEN CONCAT(SUBSTRING(e.middle_name, 1, 1), '.') ELSE '' END) AS employee_name, "
                  "CONCAT(c.last_name, ' ', c.first_name, ' ', COALESCE(c.middle_name, '')) AS client_name, "
                  "h.haircut_name "
                  "FROM documents d "
                  "JOIN branches b ON d.id_branch = b.id_branch "
                  "JOIN employees e ON d.id_employee = e.id_employee "
                  "JOIN clients c ON d.id_client = c.id_client "
                  "JOIN haircuts h ON d.id_haircut = h.id_haircut "
                  "WHERE d.id_branch = :branchId "
                  "ORDER BY d.service_date DESC, d.id_document DESC");
    query.bindValue(":branchId", branchId);

    if (query.exec()) {
        while (query.next()) {
            Document doc;
            doc.idDocument = query.value("id_document").toInt();
            doc.idBranch = query.value("id_branch").toInt();
            doc.idEmployee = query.value("id_employee").toInt();
            doc.idClient = query.value("id_client").toInt();
            doc.idHaircut = query.value("id_haircut").toInt();
            doc.serviceDate = query.value("service_date").toDateTime();
            doc.discountRate = query.value("discount_rate").toDouble();
            doc.finalCost = query.value("final_cost").toDouble();
            doc.branchName = query.value("branch_name").toString();
            doc.employeeName = query.value("employee_name").toString();
            doc.clientName = query.value("client_name").toString();
            doc.haircutName = query.value("haircut_name").toString();
            documents.append(doc);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return documents;
}

QVector<Document> DBManager::getDocumentsByClient(int clientId) {
    QVector<Document> documents;
    QSqlQuery query(m_db);
    query.prepare("SELECT d.id_document, d.id_branch, d.id_employee, d.id_client, d.id_haircut, "
                  "d.service_date, d.discount_rate, d.final_cost, "
                  "b.branch_name, "
                  "CONCAT(e.last_name, ' ', SUBSTRING(e.first_name, 1, 1), '.', "
                  "       CASE WHEN e.middle_name IS NOT NULL AND LENGTH(e.middle_name) > 0 "
                  "            THEN CONCAT(SUBSTRING(e.middle_name, 1, 1), '.') ELSE '' END) AS employee_name, "
                  "CONCAT(c.last_name, ' ', c.first_name, ' ', COALESCE(c.middle_name, '')) AS client_name, "
                  "h.haircut_name "
                  "FROM documents d "
                  "JOIN branches b ON d.id_branch = b.id_branch "
                  "JOIN employees e ON d.id_employee = e.id_employee "
                  "JOIN clients c ON d.id_client = c.id_client "
                  "JOIN haircuts h ON d.id_haircut = h.id_haircut "
                  "WHERE d.id_client = :clientId "
                  "ORDER BY d.service_date DESC, d.id_document DESC");
    query.bindValue(":clientId", clientId);

    if (query.exec()) {
        while (query.next()) {
            Document doc;
            doc.idDocument = query.value("id_document").toInt();
            doc.idBranch = query.value("id_branch").toInt();
            doc.idEmployee = query.value("id_employee").toInt();
            doc.idClient = query.value("id_client").toInt();
            doc.idHaircut = query.value("id_haircut").toInt();
            doc.serviceDate = query.value("service_date").toDateTime();
            doc.discountRate = query.value("discount_rate").toDouble();
            doc.finalCost = query.value("final_cost").toDouble();
            doc.branchName = query.value("branch_name").toString();
            doc.employeeName = query.value("employee_name").toString();
            doc.clientName = query.value("client_name").toString();
            doc.haircutName = query.value("haircut_name").toString();
            documents.append(doc);
        }
    } else {
        m_lastError = query.lastError().text();
    }
    return documents;
}

bool DBManager::deleteDocument(int docId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM documents WHERE id_document = :docId");
    query.bindValue(":docId", docId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}
