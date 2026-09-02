#include "MainWindow.h"
#include "LoginWindow.h"
#include "BranchDialog.h"
#include "HaircutDialog.h"
#include "ClientDialog.h"
#include "VisitDialog.h"
#include "UserDialog.h"
#include "PasswordChangeDialog.h"
#include "../controllers/DBManager.h"

MainWindow::MainWindow(const User &user, QWidget *parent)
    : QMainWindow(parent), m_currentUser(user) {
    if (!m_currentUser.roles.isEmpty()) {
        m_userRole = m_currentUser.roles.first().roleName;
    } else {
        m_userRole = "Client";
    }

    if (m_userRole == "Master") {
        auto empOpt = DBManager::instance().getEmployeeByUserId(m_currentUser.idUser);
        if (empOpt.has_value()) {
            m_employeeId = empOpt->idEmployee;
            m_employeeBranchId = empOpt->idBranch;
        }
    } else if (m_userRole == "Client") {
        auto clientOpt = DBManager::instance().getClientByUserId(m_currentUser.idUser);
        if (clientOpt.has_value()) {
            m_clientId = clientOpt->idClient;
        }
    }

    setWindowTitle(QString("ИС «Парикмахерская» — [%1: %2]").arg(m_userRole, m_currentUser.login));
    resize(1050, 680);
    setupUi();
}

void MainWindow::setupUi() {
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    auto *topBar = new QHBoxLayout();
    m_userInfoLabel = new QLabel(this);
    m_userInfoLabel->setText(QString("Пользователь: <b>%1</b> | Роль: <b>%2</b>").arg(m_currentUser.login, m_userRole));
    topBar->addWidget(m_userInfoLabel);

    topBar->addStretch();

    auto *logoutBtn = new QPushButton("Сменить пользователя", this);
    logoutBtn->setStyleSheet("background-color: #e2e8f0; border: 1px solid #cbd5e0; padding: 4px 12px; border-radius: 4px;");
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);
    topBar->addWidget(logoutBtn);

    mainLayout->addLayout(topBar);

    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    setCentralWidget(central);
    statusBar()->showMessage("Готово");

    if (m_userRole == "Admin") {
        setupAdminUi();
    } else if (m_userRole == "Master") {
        setupMasterUi();
    } else {
        setupClientUi();
    }
}

void MainWindow::setupAdminUi() {
    m_tabWidget->addTab(createBranchesTab(), "Филиалы");
    m_tabWidget->addTab(createHaircutsTab(), "Каталог стрижек");
    m_tabWidget->addTab(createVisitsTab(false), "Журнал визитов");
    m_tabWidget->addTab(createClientsTab(), "Клиенты и скидки");
    m_tabWidget->addTab(createUsersTab(), "Пользователи");
}

void MainWindow::setupMasterUi() {
    m_tabWidget->addTab(createVisitsTab(true), "Визиты филиала");
    m_tabWidget->addTab(createClientsTab(), "Клиентская база");
    m_tabWidget->addTab(createHaircutsTab(), "Прейскурант услуг");
}

void MainWindow::setupClientUi() {
    m_tabWidget->addTab(createClientCabinetTab(), "Личный кабинет");
    m_tabWidget->addTab(createHaircutsTab(), "Каталог стрижек");
}

QWidget* MainWindow::createBranchesTab() {
    auto *tab = new QWidget(this);
    return tab;
}

QWidget* MainWindow::createHaircutsTab() {
    auto *tab = new QWidget(this);
    return tab;
}

QWidget* MainWindow::createUsersTab() {
    auto *tab = new QWidget(this);
    return tab;
}

QWidget* MainWindow::createClientsTab() {
    auto *tab = new QWidget(this);
    return tab;
}

QWidget* MainWindow::createVisitsTab(bool) {
    auto *tab = new QWidget(this);
    return tab;
}

QWidget* MainWindow::createClientCabinetTab() {
    auto *tab = new QWidget(this);
    return tab;
}

void MainWindow::onLogoutClicked() {
    emit logoutRequested();
}

void MainWindow::onAddBranch() {}
void MainWindow::onEditBranch() {}
void MainWindow::onDeleteBranch() {}
void MainWindow::onRefreshBranches() {}

void MainWindow::onAddHaircut() {}
void MainWindow::onEditHaircut() {}
void MainWindow::onDeleteHaircut() {}
void MainWindow::onChangeHaircutPrice() {}
void MainWindow::onHaircutSelectionChanged() {}
void MainWindow::onRefreshHaircuts() {}

void MainWindow::onAddUser() {}
void MainWindow::onDeleteUser() {}
void MainWindow::onResetUserPassword() {}
void MainWindow::onRefreshUsers() {}

void MainWindow::onAddClient() {}
void MainWindow::onEditClient() {}
void MainWindow::onDeleteClient() {}
void MainWindow::onRefreshClients() {}

void MainWindow::onAddVisit() {}
void MainWindow::onDeleteVisit() {}
void MainWindow::onRefreshVisits() {}
void MainWindow::onFilterVisits() {}

void MainWindow::onEditClientPhone() {}
void MainWindow::onChangeClientPassword() {}
void MainWindow::onRefreshClientProfile() {}

void MainWindow::loadBranches() {}
void MainWindow::loadHaircuts() {}
void MainWindow::loadUsers() {}
void MainWindow::loadClients() {}
void MainWindow::loadVisits() {}
void MainWindow::loadClientProfile() {}
