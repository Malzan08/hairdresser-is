#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStatusBar>
#include <QMessageBox>
#include "../models/User.h"
#include "../models/Employee.h"
#include "../models/Client.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const User &user, QWidget *parent = nullptr);
    ~MainWindow() override = default;

signals:
    void logoutRequested();

private slots:
    void onLogoutClicked();

    void onAddBranch();
    void onEditBranch();
    void onDeleteBranch();
    void onRefreshBranches();

    void onAddHaircut();
    void onEditHaircut();
    void onDeleteHaircut();
    void onChangeHaircutPrice();
    void onHaircutSelectionChanged();
    void onRefreshHaircuts();

    void onAddUser();
    void onDeleteUser();
    void onResetUserPassword();
    void onRefreshUsers();

    void onAddClient();
    void onEditClient();
    void onDeleteClient();
    void onRefreshClients();

    void onAddVisit();
    void onDeleteVisit();
    void onRefreshVisits();
    void onFilterVisits();

    void onEditClientPhone();
    void onChangeClientPassword();
    void onRefreshClientProfile();

private:
    void setupUi();
    void setupAdminUi();
    void setupMasterUi();
    void setupClientUi();

    QWidget* createBranchesTab();
    QWidget* createHaircutsTab();
    QWidget* createUsersTab();
    QWidget* createClientsTab();
    QWidget* createVisitsTab(bool isMasterView = false);
    QWidget* createClientCabinetTab();

    void loadBranches();
    void loadHaircuts();
    void loadUsers();
    void loadClients();
    void loadVisits();
    void loadClientProfile();

    User m_currentUser;
    QString m_userRole;
    int m_employeeId{0};
    int m_employeeBranchId{0};
    int m_clientId{0};

    QTabWidget *m_tabWidget{nullptr};
    QLabel *m_userInfoLabel{nullptr};

    QTableWidget *m_branchesTable{nullptr};
    QTableWidget *m_branchReportsTable{nullptr};

    QTableWidget *m_haircutsTable{nullptr};
    QTableWidget *m_priceHistoryTable{nullptr};

    QTableWidget *m_usersTable{nullptr};

    QTableWidget *m_clientsTable{nullptr};

    QTableWidget *m_visitsTable{nullptr};
    QComboBox *m_visitBranchFilter{nullptr};
    QPushButton *m_addVisitBtn{nullptr};
    QPushButton *m_deleteVisitBtn{nullptr};

    QLabel *m_cabinetNameLabel{nullptr};
    QLabel *m_cabinetPhoneLabel{nullptr};
    QLabel *m_cabinetVisitsLabel{nullptr};
    QLabel *m_cabinetDiscountLabel{nullptr};
    QLabel *m_cabinetSpentLabel{nullptr};
    QTableWidget *m_cabinetVisitsTable{nullptr};
};

#endif
