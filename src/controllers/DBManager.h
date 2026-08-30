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
    bool updateUserPassword(int userId, const QString &newPassword);
    QVector<User> getAllUsers();
    bool deleteUser(int userId);
    bool setUserRole(int userId, int roleId);

    QVector<Branch> getAllBranches();
    std::optional<Branch> getBranchById(int branchId);
    bool addBranch(const QString &name, const QString &address, const QString &phone, int *outBranchId = nullptr);
    bool updateBranch(int branchId, const QString &name, const QString &address, const QString &phone);
    bool deleteBranch(int branchId);

    QVector<Employee> getAllEmployees();
    QVector<Employee> getEmployeesByBranch(int branchId);
    std::optional<Employee> getEmployeeByUserId(int userId);
    std::optional<Employee> getEmployeeById(int employeeId);

    QVector<Haircut> getAllHaircuts();
    std::optional<Haircut> getHaircutById(int haircutId);
    bool addHaircut(const QString &name, const QString &gender, double initialPrice, int *outHaircutId = nullptr);
    bool updateHaircut(int haircutId, const QString &name, const QString &gender);
    bool deleteHaircut(int haircutId);

    bool addPriceHistory(int haircutId, double newPrice);
    QVector<PriceHistory> getPriceHistoryByHaircut(int haircutId);

    QVector<Client> getAllClients();
    std::optional<Client> getClientById(int clientId);
    std::optional<Client> getClientByPhone(const QString &phone);
    std::optional<Client> getClientByUserId(int userId);
    bool addClient(const QString &lastName, const QString &firstName, const QString &middleName, const QString &phone, std::optional<int> userId = std::nullopt, int *outClientId = nullptr);
    bool updateClientPhone(int clientId, const QString &newPhone);
    bool updateClient(int clientId, const QString &lastName, const QString &firstName, const QString &middleName, const QString &phone);
    bool deleteClient(int clientId);

    bool addDocument(int branchId, int employeeId, int clientId, int haircutId, const QDateTime &serviceDate = QDateTime::currentDateTime(), int *outDocId = nullptr);
    QVector<Document> getAllDocuments();
    QVector<Document> getDocumentsByBranch(int branchId);
    QVector<Document> getDocumentsByClient(int clientId);
    bool deleteDocument(int docId);

    QVector<ClientStats> getClientStats();
    QVector<BranchReport> getBranchReports();
    int getClientVisitCount(int clientId);
    double getClientDiscount(int clientId);
    double getClientTotalSpent(int clientId);

private:
    DBManager(QObject *parent = nullptr);
    ~DBManager() override;
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    QSqlDatabase m_db;
    QString m_lastError;
};

#endif
