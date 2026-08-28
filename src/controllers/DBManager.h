#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVector>
#include <QVariant>
#include <optional>

#include "../models/Role.h"
#include "../models/User.h"
#include "../models/Branch.h"
#include "../models/Employee.h"
#include "../models/Client.h"
#include "../models/Haircut.h"
#include "../models/PriceHistory.h"
#include "../models/Document.h"
#include "../models/ReportStats.h"

class DBManager : public QObject {
    Q_OBJECT

public:
    static DBManager& instance();

    bool connectDatabase(const QString &host = "127.0.0.1",
                         int port = 5432,
                         const QString &dbName = "hairdresser_db",
                         const QString &user = "postgres",
                         const QString &password = "admin");
    void disconnectDatabase();
    bool isOpen() const;
    QString lastError() const;

    std::optional<User> authenticateUser(const QString &login, const QString &password);
    QVector<Role> getUserRoles(int userId);
    std::optional<User> getUserByLogin(const QString &login);
    std::optional<User> getUserById(int userId);
    bool checkLoginExists(const QString &login);
    bool registerUser(const QString &login, const QString &password, const QString &roleName, int *outUserId = nullptr);
    QVector<Role> getAllRoles();

private:
    DBManager(QObject *parent = nullptr);
    ~DBManager() override;
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    QSqlDatabase m_db;
    QString m_lastError;
};

#endif
