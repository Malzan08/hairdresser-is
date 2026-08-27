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
