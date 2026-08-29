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
