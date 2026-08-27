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
