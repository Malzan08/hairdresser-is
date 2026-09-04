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
    QString userDetails = QString("Пользователь: <b>%1</b> | Роль: <b>%2</b>").arg(m_currentUser.login, m_userRole);
    if (m_userRole == "Master" && m_employeeBranchId > 0) {
        auto branchOpt = DBManager::instance().getBranchById(m_employeeBranchId);
        if (branchOpt.has_value()) {
            userDetails += QString(" | Филиал: <b>%1</b>").arg(branchOpt->branchName);
        }
    }
    m_userInfoLabel->setText(userDetails);
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
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    auto *btnLayout = new QHBoxLayout();
    auto *addBtn = new QPushButton("Добавить филиал", tab);
    auto *editBtn = new QPushButton("Редактировать", tab);
    auto *delBtn = new QPushButton("Удалить филиал", tab);
    auto *refreshBtn = new QPushButton("Обновить", tab);

    addBtn->setStyleSheet("background-color: #3182ce; color: white; padding: 5px 12px;");
    delBtn->setStyleSheet("background-color: #e53e3e; color: white; padding: 5px 12px;");

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddBranch);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::onEditBranch);
    connect(delBtn, &QPushButton::clicked, this, &MainWindow::onDeleteBranch);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshBranches);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(delBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(refreshBtn);
    layout->addLayout(btnLayout);

    auto *splitLayout = new QHBoxLayout();

    auto *leftBox = new QVBoxLayout();
    auto *branchesLabel = new QLabel("<b>Список филиалов сети</b>", tab);
    leftBox->addWidget(branchesLabel);

    m_branchesTable = new QTableWidget(tab);
    m_branchesTable->setColumnCount(4);
    m_branchesTable->setHorizontalHeaderLabels({"ID", "Название", "Адрес", "Телефон"});
    m_branchesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_branchesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_branchesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_branchesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftBox->addWidget(m_branchesTable);
    splitLayout->addLayout(leftBox, 3);

    auto *rightBox = new QVBoxLayout();
    auto *reportLabel = new QLabel("<b>Финансовая сводка по филиалам (View)</b>", tab);
    rightBox->addWidget(reportLabel);

    m_branchReportsTable = new QTableWidget(tab);
    m_branchReportsTable->setColumnCount(4);
    m_branchReportsTable->setHorizontalHeaderLabels({"ID", "Филиал", "Услуг", "Выручка (руб.)"});
    m_branchReportsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_branchReportsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_branchReportsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rightBox->addWidget(m_branchReportsTable);
    splitLayout->addLayout(rightBox, 2);

    layout->addLayout(splitLayout);

    loadBranches();
    return tab;
}

QWidget* MainWindow::createHaircutsTab() {
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    auto *btnLayout = new QHBoxLayout();
    auto *addBtn = new QPushButton("Добавить стрижку", tab);
    auto *editBtn = new QPushButton("Редактировать", tab);
    auto *priceBtn = new QPushButton("Изменить цену", tab);
    auto *delBtn = new QPushButton("Удалить стрижку", tab);
    auto *refreshBtn = new QPushButton("Обновить", tab);

    addBtn->setStyleSheet("background-color: #3182ce; color: white; padding: 5px 12px;");
    priceBtn->setStyleSheet("background-color: #d69e2e; color: white; padding: 5px 12px; font-weight: bold;");
    delBtn->setStyleSheet("background-color: #e53e3e; color: white; padding: 5px 12px;");

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddHaircut);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::onEditHaircut);
    connect(priceBtn, &QPushButton::clicked, this, &MainWindow::onChangeHaircutPrice);
    connect(delBtn, &QPushButton::clicked, this, &MainWindow::onDeleteHaircut);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshHaircuts);

    if (m_userRole == "Admin") {
        btnLayout->addWidget(addBtn);
        btnLayout->addWidget(editBtn);
        btnLayout->addWidget(priceBtn);
        btnLayout->addWidget(delBtn);
    }
    btnLayout->addStretch();
    btnLayout->addWidget(refreshBtn);
    layout->addLayout(btnLayout);

    auto *splitLayout = new QHBoxLayout();

    auto *leftBox = new QVBoxLayout();
    auto *hLabel = new QLabel("<b>Каталог услуг и стрижек</b>", tab);
    leftBox->addWidget(hLabel);

    m_haircutsTable = new QTableWidget(tab);
    m_haircutsTable->setColumnCount(4);
    m_haircutsTable->setHorizontalHeaderLabels({"ID", "Название услуги", "Категория", "Текущая цена"});
    m_haircutsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_haircutsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_haircutsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_haircutsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_haircutsTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onHaircutSelectionChanged);
    leftBox->addWidget(m_haircutsTable);
    splitLayout->addLayout(leftBox, 3);

    auto *rightBox = new QVBoxLayout();
    auto *histLabel = new QLabel("<b>История изменения цен (Price History)</b>", tab);
    rightBox->addWidget(histLabel);

    m_priceHistoryTable = new QTableWidget(tab);
    m_priceHistoryTable->setColumnCount(3);
    m_priceHistoryTable->setHorizontalHeaderLabels({"ID", "Цена (руб.)", "Дата и время фиксации"});
    m_priceHistoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_priceHistoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_priceHistoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    rightBox->addWidget(m_priceHistoryTable);
    splitLayout->addLayout(rightBox, 2);

    layout->addLayout(splitLayout);

    loadHaircuts();
    return tab;
}

QWidget* MainWindow::createUsersTab() {
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    auto *btnLayout = new QHBoxLayout();
    auto *addBtn = new QPushButton("Добавить пользователя", tab);
    auto *pwdBtn = new QPushButton("Сменить пароль", tab);
    auto *delBtn = new QPushButton("Удалить пользователя", tab);
    auto *refreshBtn = new QPushButton("Обновить", tab);

    addBtn->setStyleSheet("background-color: #3182ce; color: white; padding: 5px 12px;");
    delBtn->setStyleSheet("background-color: #e53e3e; color: white; padding: 5px 12px;");

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddUser);
    connect(pwdBtn, &QPushButton::clicked, this, &MainWindow::onResetUserPassword);
    connect(delBtn, &QPushButton::clicked, this, &MainWindow::onDeleteUser);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshUsers);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(pwdBtn);
    btnLayout->addWidget(delBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(refreshBtn);
    layout->addLayout(btnLayout);

    m_usersTable = new QTableWidget(tab);
    m_usersTable->setColumnCount(3);
    m_usersTable->setHorizontalHeaderLabels({"ID", "Логин", "Роль в системе"});
    m_usersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_usersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_usersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_usersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_usersTable);

    loadUsers();
    return tab;
}

QWidget* MainWindow::createClientsTab() {
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    auto *btnLayout = new QHBoxLayout();
    auto *addBtn = new QPushButton("Добавить клиента", tab);
    auto *editBtn = new QPushButton("Редактировать", tab);
    auto *delBtn = new QPushButton("Удалить", tab);
    auto *refreshBtn = new QPushButton("Обновить", tab);

    addBtn->setStyleSheet("background-color: #3182ce; color: white; padding: 5px 12px;");
    delBtn->setStyleSheet("background-color: #e53e3e; color: white; padding: 5px 12px;");

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddClient);
    connect(editBtn, &QPushButton::clicked, this, &MainWindow::onEditClient);
    connect(delBtn, &QPushButton::clicked, this, &MainWindow::onDeleteClient);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClients);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    if (m_userRole == "Admin") {
        btnLayout->addWidget(delBtn);
    }
    btnLayout->addStretch();
    btnLayout->addWidget(refreshBtn);
    layout->addLayout(btnLayout);

    auto *infoLabel = new QLabel("<b>Клиентская база и статистика программы лояльности (View: view_client_stats)</b>", tab);
    layout->addWidget(infoLabel);

    m_clientsTable = new QTableWidget(tab);
    m_clientsTable->setColumnCount(6);
    m_clientsTable->setHorizontalHeaderLabels({"ID", "ФИО клиента", "Телефон", "Всего визитов", "Статус клиента", "Потрачено (руб.)"});
    m_clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_clientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_clientsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_clientsTable);

    loadClients();
    return tab;
}

QWidget* MainWindow::createVisitsTab(bool isMasterView) {
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    auto *topLayout = new QHBoxLayout();
    m_addVisitBtn = new QPushButton("Оформить визит", tab);
    m_addVisitBtn->setStyleSheet("background-color: #38a169; color: white; font-weight: bold; padding: 5px 14px;");
    connect(m_addVisitBtn, &QPushButton::clicked, this, &MainWindow::onAddVisit);
    topLayout->addWidget(m_addVisitBtn);

    if (!isMasterView && m_userRole == "Admin") {
        m_deleteVisitBtn = new QPushButton("Удалить запись", tab);
        m_deleteVisitBtn->setStyleSheet("background-color: #e53e3e; color: white; padding: 5px 12px;");
        connect(m_deleteVisitBtn, &QPushButton::clicked, this, &MainWindow::onDeleteVisit);
        topLayout->addWidget(m_deleteVisitBtn);

        topLayout->addSpacing(15);
        topLayout->addWidget(new QLabel("Фильтр по филиалу:", tab));
        m_visitBranchFilter = new QComboBox(tab);
        m_visitBranchFilter->addItem("Все филиалы", 0);
        auto branches = DBManager::instance().getAllBranches();
        for (const auto &b : branches) {
            m_visitBranchFilter->addItem(b.branchName, b.idBranch);
        }
        connect(m_visitBranchFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFilterVisits);
        topLayout->addWidget(m_visitBranchFilter);
    }

    topLayout->addStretch();

    auto *refreshBtn = new QPushButton("Обновить журнал", tab);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshVisits);
    topLayout->addWidget(refreshBtn);

    layout->addLayout(topLayout);

    m_visitsTable = new QTableWidget(tab);
    m_visitsTable->setColumnCount(8);
    m_visitsTable->setHorizontalHeaderLabels({"ID", "Дата и время", "Филиал", "Мастер", "Клиент", "Услуга", "Скидка", "Итог к оплате"});
    m_visitsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_visitsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_visitsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_visitsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_visitsTable);

    loadVisits();
    return tab;
}

QWidget* MainWindow::createClientCabinetTab() {
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(12);

    auto *cardWidget = new QWidget(tab);
    cardWidget->setStyleSheet("background-color: #f7fafc; border: 1px solid #e2e8f0; border-radius: 8px;");
    auto *cardLayout = new QVBoxLayout(cardWidget);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    cardLayout->setSpacing(8);

    auto *cardTitle = new QLabel("<b>Личные данные и статус программы лояльности</b>", cardWidget);
    cardTitle->setStyleSheet("font-size: 14px; color: #2d3748; border: none;");
    cardLayout->addWidget(cardTitle);

    auto *infoGrid = new QGridLayout();
    infoGrid->setSpacing(10);

    m_cabinetNameLabel = new QLabel(cardWidget);
    m_cabinetNameLabel->setStyleSheet("border: none; font-size: 13px;");
    infoGrid->addWidget(new QLabel("ФИО:", cardWidget), 0, 0);
    infoGrid->addWidget(m_cabinetNameLabel, 0, 1);

    m_cabinetPhoneLabel = new QLabel(cardWidget);
    m_cabinetPhoneLabel->setStyleSheet("border: none; font-size: 13px;");
    infoGrid->addWidget(new QLabel("Номер телефона:", cardWidget), 1, 0);
    infoGrid->addWidget(m_cabinetPhoneLabel, 1, 1);

    m_cabinetVisitsLabel = new QLabel(cardWidget);
    m_cabinetVisitsLabel->setStyleSheet("border: none; font-size: 13px; font-weight: bold; color: #3182ce;");
    infoGrid->addWidget(new QLabel("Всего визитов:", cardWidget), 0, 2);
    infoGrid->addWidget(m_cabinetVisitsLabel, 0, 3);

    m_cabinetDiscountLabel = new QLabel(cardWidget);
    m_cabinetDiscountLabel->setStyleSheet("border: none; font-size: 13px; font-weight: bold;");
    infoGrid->addWidget(new QLabel("Текущая скидка:", cardWidget), 1, 2);
    infoGrid->addWidget(m_cabinetDiscountLabel, 1, 3);

    m_cabinetSpentLabel = new QLabel(cardWidget);
    m_cabinetSpentLabel->setStyleSheet("border: none; font-size: 13px; font-weight: bold; color: #276749;");
    infoGrid->addWidget(new QLabel("Потрачено за все время:", cardWidget), 2, 2);
    infoGrid->addWidget(m_cabinetSpentLabel, 2, 3);

    cardLayout->addLayout(infoGrid);

    auto *actionsLayout = new QHBoxLayout();
    auto *editPhoneBtn = new QPushButton("Изменить телефон", cardWidget);
    editPhoneBtn->setStyleSheet("background-color: #4a5568; color: white; padding: 4px 12px; border-radius: 4px;");
    connect(editPhoneBtn, &QPushButton::clicked, this, &MainWindow::onEditClientPhone);
    actionsLayout->addWidget(editPhoneBtn);

    auto *changePwdBtn = new QPushButton("Сменить пароль", cardWidget);
    changePwdBtn->setStyleSheet("background-color: #4a5568; color: white; padding: 4px 12px; border-radius: 4px;");
    connect(changePwdBtn, &QPushButton::clicked, this, &MainWindow::onChangeClientPassword);
    actionsLayout->addWidget(changePwdBtn);

    actionsLayout->addStretch();

    auto *refreshBtn = new QPushButton("Обновить данные", cardWidget);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshClientProfile);
    actionsLayout->addWidget(refreshBtn);

    cardLayout->addLayout(actionsLayout);
    layout->addWidget(cardWidget);

    auto *historyTitle = new QLabel("<b>История моих стрижек и визитов</b>", tab);
    historyTitle->setStyleSheet("font-size: 13px; color: #2d3748;");
    layout->addWidget(historyTitle);

    m_cabinetVisitsTable = new QTableWidget(tab);
    m_cabinetVisitsTable->setColumnCount(6);
    m_cabinetVisitsTable->setHorizontalHeaderLabels({"Дата и время", "Филиал", "Мастер", "Стрижка / Услуга", "Скидка", "Итоговая цена"});
    m_cabinetVisitsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_cabinetVisitsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_cabinetVisitsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_cabinetVisitsTable);

    loadClientProfile();
    return tab;
}

void MainWindow::onLogoutClicked() {
    emit logoutRequested();
}

void MainWindow::onAddBranch() {
    BranchDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        if (!DBManager::instance().addBranch(dlg.branchName(), dlg.address(), dlg.phone())) {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить филиал: " + DBManager::instance().lastError());
        } else {
            loadBranches();
            statusBar()->showMessage("Филиал успешно добавлен", 3000);
        }
    }
}

void MainWindow::onEditBranch() {
    int row = m_branchesTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите филиал для редактирования.");
        return;
    }

    Branch b;
    b.idBranch = m_branchesTable->item(row, 0)->text().toInt();
    b.branchName = m_branchesTable->item(row, 1)->text();
    b.address = m_branchesTable->item(row, 2)->text();
    b.phone = m_branchesTable->item(row, 3)->text();

    BranchDialog dlg(b, this);
    if (dlg.exec() == QDialog::Accepted) {
        if (!DBManager::instance().updateBranch(b.idBranch, dlg.branchName(), dlg.address(), dlg.phone())) {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить филиал: " + DBManager::instance().lastError());
        } else {
            loadBranches();
            statusBar()->showMessage("Данные филиала обновлены", 3000);
        }
    }
}

void MainWindow::onDeleteBranch() {
    int row = m_branchesTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите филиал для удаления.");
        return;
    }

    int branchId = m_branchesTable->item(row, 0)->text().toInt();
    QString name = m_branchesTable->item(row, 1)->text();

    if (QMessageBox::question(this, "Подтверждение", "Удалить филиал «" + name + "»? Это может затронуть связанные записи.") == QMessageBox::Yes) {
        if (!DBManager::instance().deleteBranch(branchId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить филиал: " + DBManager::instance().lastError());
        } else {
            loadBranches();
            statusBar()->showMessage("Филиал удален", 3000);
        }
    }
}

void MainWindow::onRefreshBranches() {
    loadBranches();
    statusBar()->showMessage("Список филиалов обновлен", 2000);
}

void MainWindow::onAddHaircut() {
    HaircutDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        if (!DBManager::instance().addHaircut(dlg.haircutName(), dlg.gender(), dlg.price())) {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить услугу: " + DBManager::instance().lastError());
        } else {
            loadHaircuts();
            statusBar()->showMessage("Стрижка успешно добавлена", 3000);
        }
    }
}

void MainWindow::onEditHaircut() {
    int row = m_haircutsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите услугу для редактирования.");
        return;
    }

    Haircut h;
    h.idHaircut = m_haircutsTable->item(row, 0)->text().toInt();
    h.haircutName = m_haircutsTable->item(row, 1)->text();
    h.gender = m_haircutsTable->item(row, 2)->text();
    h.currentPrice = m_haircutsTable->item(row, 3)->text().split(" ").first().toDouble();

    HaircutDialog dlg(h, this);
    if (dlg.exec() == QDialog::Accepted) {
        if (!DBManager::instance().updateHaircut(h.idHaircut, dlg.haircutName(), dlg.gender())) {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить стрижку: " + DBManager::instance().lastError());
        } else {
            loadHaircuts();
            statusBar()->showMessage("Параметры стрижки обновлены", 3000);
        }
    }
}

void MainWindow::onDeleteHaircut() {
    int row = m_haircutsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите стрижку для удаления.");
        return;
    }

    int haircutId = m_haircutsTable->item(row, 0)->text().toInt();
    QString name = m_haircutsTable->item(row, 1)->text();

    if (QMessageBox::question(this, "Подтверждение", "Удалить услугу «" + name + "»? Все связанные записи из истории цен также будут удалены.") == QMessageBox::Yes) {
        if (!DBManager::instance().deleteHaircut(haircutId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить стрижку: " + DBManager::instance().lastError());
        } else {
            loadHaircuts();
            statusBar()->showMessage("Стрижка удалена", 3000);
        }
    }
}

void MainWindow::onChangeHaircutPrice() {
    int row = m_haircutsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите стрижку для изменения стоимости.");
        return;
    }

    int haircutId = m_haircutsTable->item(row, 0)->text().toInt();
    QString name = m_haircutsTable->item(row, 1)->text();
    double currentPrice = m_haircutsTable->item(row, 3)->text().split(" ").first().toDouble();

    QDialog dlg(this);
    dlg.setWindowTitle("Изменение стоимости стрижки");
    dlg.setFixedWidth(360);
    auto *l = new QVBoxLayout(&dlg);
    l->setSpacing(12);

    auto *info = new QLabel(QString("Услуга: <b>%1</b><br>Текущая цена: %2 ₽").arg(name).arg(currentPrice, 0, 'f', 2), &dlg);
    l->addWidget(info);

    auto *spin = new QDoubleSpinBox(&dlg);
    spin->setRange(100.0, 50000.0);
    spin->setSingleStep(50.0);
    spin->setValue(currentPrice);
    spin->setSuffix(" ₽");
    l->addWidget(new QLabel("Новая стоимость:", &dlg));
    l->addWidget(spin);

    auto *hl = new QHBoxLayout();
    auto *cBtn = new QPushButton("Отмена", &dlg);
    auto *sBtn = new QPushButton("Зафиксировать в истории", &dlg);
    sBtn->setStyleSheet("background-color: #3182ce; color: white; font-weight: bold; padding: 6px 12px;");
    connect(cBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(sBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    hl->addStretch();
    hl->addWidget(cBtn);
    hl->addWidget(sBtn);
    l->addLayout(hl);

    if (dlg.exec() == QDialog::Accepted) {
        double newPrice = spin->value();
        if (!DBManager::instance().addPriceHistory(haircutId, newPrice)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось зафиксировать новую цену: " + DBManager::instance().lastError());
        } else {
            loadHaircuts();
            statusBar()->showMessage("Новая цена успешно зафиксирована в истории цен", 3000);
        }
    }
}

void MainWindow::onHaircutSelectionChanged() {
    int row = m_haircutsTable->currentRow();
    if (row < 0 || !m_priceHistoryTable) return;

    int haircutId = m_haircutsTable->item(row, 0)->text().toInt();
    auto history = DBManager::instance().getPriceHistoryByHaircut(haircutId);
    m_priceHistoryTable->setRowCount(history.size());
    for (int i = 0; i < history.size(); ++i) {
        m_priceHistoryTable->setItem(i, 0, new QTableWidgetItem(QString::number(history[i].idPriceHistory)));
        m_priceHistoryTable->setItem(i, 1, new QTableWidgetItem(QString("%1 ₽").arg(history[i].price, 0, 'f', 2)));
        m_priceHistoryTable->setItem(i, 2, new QTableWidgetItem(history[i].changeDate.toString("dd.MM.yyyy HH:mm:ss")));
    }
}

void MainWindow::onRefreshHaircuts() {
    loadHaircuts();
    statusBar()->showMessage("Каталог стрижек обновлен", 2000);
}

void MainWindow::onAddUser() {
    UserDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        int newUserId = 0;
        if (!DBManager::instance().registerUser(dlg.login(), dlg.password(), dlg.roleName(), &newUserId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать пользователя: " + DBManager::instance().lastError());
        } else {
            loadUsers();
            statusBar()->showMessage("Пользователь успешно создан", 3000);
        }
    }
}

void MainWindow::onDeleteUser() {
    int row = m_usersTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите пользователя для удаления.");
        return;
    }

    int userId = m_usersTable->item(row, 0)->text().toInt();
    QString login = m_usersTable->item(row, 1)->text();

    if (userId == m_currentUser.idUser) {
        QMessageBox::warning(this, "Предупреждение", "Нельзя удалить текущую учетную запись.");
        return;
    }

    if (QMessageBox::question(this, "Подтверждение", "Удалить пользователя «" + login + "»?") == QMessageBox::Yes) {
        if (!DBManager::instance().deleteUser(userId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить пользователя: " + DBManager::instance().lastError());
        } else {
            loadUsers();
            statusBar()->showMessage("Пользователь удален", 3000);
        }
    }
}

void MainWindow::onResetUserPassword() {
    int row = m_usersTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите пользователя для смены пароля.");
        return;
    }

    int userId = m_usersTable->item(row, 0)->text().toInt();
    QString login = m_usersTable->item(row, 1)->text();

    PasswordChangeDialog dlg(this);
    dlg.setWindowTitle("Смена пароля: " + login);
    if (dlg.exec() == QDialog::Accepted) {
        if (!DBManager::instance().updateUserPassword(userId, dlg.newPassword())) {
            QMessageBox::critical(this, "Ошибка", "Не удалось сменить пароль: " + DBManager::instance().lastError());
        } else {
            statusBar()->showMessage("Пароль пользователя успешно изменен", 3000);
        }
    }
}

void MainWindow::onRefreshUsers() {
    loadUsers();
    statusBar()->showMessage("Список пользователей обновлен", 2000);
}

void MainWindow::onAddClient() {
    ClientDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        if (!DBManager::instance().addClient(dlg.lastName(), dlg.firstName(), dlg.middleName(), dlg.phone())) {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить клиента: " + DBManager::instance().lastError());
        } else {
            loadClients();
            statusBar()->showMessage("Клиент успешно зарегистрирован", 3000);
        }
    }
}

void MainWindow::onEditClient() {
    int row = m_clientsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите клиента для редактирования.");
        return;
    }

    int clientId = m_clientsTable->item(row, 0)->text().toInt();
    auto clientOpt = DBManager::instance().getClientById(clientId);
    if (!clientOpt.has_value()) return;

    ClientDialog dlg(clientOpt.value(), this);
    if (dlg.exec() == QDialog::Accepted) {
        if (!DBManager::instance().updateClient(clientId, dlg.lastName(), dlg.firstName(), dlg.middleName(), dlg.phone())) {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить данные клиента: " + DBManager::instance().lastError());
        } else {
            loadClients();
            statusBar()->showMessage("Данные клиента обновлены", 3000);
        }
    }
}

void MainWindow::onDeleteClient() {
    int row = m_clientsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите клиента для удаления.");
        return;
    }

    int clientId = m_clientsTable->item(row, 0)->text().toInt();
    QString name = m_clientsTable->item(row, 1)->text();

    if (QMessageBox::question(this, "Подтверждение", "Удалить клиента «" + name + "»?") == QMessageBox::Yes) {
        if (!DBManager::instance().deleteClient(clientId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить клиента: " + DBManager::instance().lastError());
        } else {
            loadClients();
            statusBar()->showMessage("Клиент удален", 3000);
        }
    }
}

void MainWindow::onRefreshClients() {
    loadClients();
    statusBar()->showMessage("Список клиентов обновлен", 2000);
}

void MainWindow::onAddVisit() {
    VisitDialog dlg(m_employeeBranchId, m_employeeId, this);
    if (dlg.exec() == QDialog::Accepted) {
        int docId = 0;
        if (!DBManager::instance().addDocument(dlg.selectedBranchId(), dlg.selectedEmployeeId(),
                                               dlg.selectedClientId(), dlg.selectedHaircutId(),
                                               dlg.selectedDateTime(), &docId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось оформить визит: " + DBManager::instance().lastError());
        } else {
            loadVisits();
            if (m_clientsTable) loadClients();
            if (m_branchReportsTable) loadBranches();
            statusBar()->showMessage(QString("Визит №%1 успешно оформлен!").arg(docId), 3000);
        }
    }
}

void MainWindow::onDeleteVisit() {
    int row = m_visitsTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Внимание", "Выберите запись визита для удаления.");
        return;
    }

    int docId = m_visitsTable->item(row, 0)->text().toInt();

    if (QMessageBox::question(this, "Подтверждение", QString("Удалить запись визита №%1?").arg(docId)) == QMessageBox::Yes) {
        if (!DBManager::instance().deleteDocument(docId)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить запись: " + DBManager::instance().lastError());
        } else {
            loadVisits();
            if (m_clientsTable) loadClients();
            if (m_branchReportsTable) loadBranches();
            statusBar()->showMessage("Запись визита удалена", 3000);
        }
    }
}

void MainWindow::onRefreshVisits() {
    loadVisits();
    statusBar()->showMessage("Журнал визитов обновлен", 2000);
}

void MainWindow::onFilterVisits() {
    loadVisits();
}

void MainWindow::onEditClientPhone() {
    if (m_clientId <= 0) return;
    auto clientOpt = DBManager::instance().getClientById(m_clientId);
    if (!clientOpt.has_value()) return;

    QDialog dlg(this);
    dlg.setWindowTitle("Изменение номера телефона");
    dlg.setFixedWidth(360);
    auto *l = new QVBoxLayout(&dlg);
    l->setSpacing(12);

    auto *edit = new QLineEdit(&dlg);
    edit->setText(clientOpt->phone);
    edit->setPlaceholderText("+7(999)000-00-00");
    l->addWidget(new QLabel("Новый номер телефона:", &dlg));
    l->addWidget(edit);

    auto *hl = new QHBoxLayout();
    auto *cBtn = new QPushButton("Отмена", &dlg);
    auto *sBtn = new QPushButton("Сохранить", &dlg);
    sBtn->setStyleSheet("background-color: #3182ce; color: white; font-weight: bold; padding: 6px 12px;");
    connect(cBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(sBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    hl->addStretch();
    hl->addWidget(cBtn);
    hl->addWidget(sBtn);
    l->addLayout(hl);

    if (dlg.exec() == QDialog::Accepted) {
        QString newPhone = edit->text().trimmed();
        if (newPhone.isEmpty()) {
            QMessageBox::warning(this, "Внимание", "Номер телефона не может быть пустым.");
            return;
        }
        if (!DBManager::instance().updateClientPhone(m_clientId, newPhone)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить телефон: " + DBManager::instance().lastError());
        } else {
            loadClientProfile();
            statusBar()->showMessage("Телефон успешно обновлен", 3000);
        }
    }
}

void MainWindow::onChangeClientPassword() {
    PasswordChangeDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        if (!DBManager::instance().updateUserPassword(m_currentUser.idUser, dlg.newPassword())) {
            QMessageBox::critical(this, "Ошибка", "Не удалось сменить пароль: " + DBManager::instance().lastError());
        } else {
            statusBar()->showMessage("Пароль успешно изменен", 3000);
        }
    }
}

void MainWindow::onRefreshClientProfile() {
    loadClientProfile();
    statusBar()->showMessage("Данные профиля обновлены", 2000);
}

void MainWindow::loadBranches() {
    if (!m_branchesTable) return;
    auto branches = DBManager::instance().getAllBranches();
    m_branchesTable->setRowCount(branches.size());
    for (int i = 0; i < branches.size(); ++i) {
        m_branchesTable->setItem(i, 0, new QTableWidgetItem(QString::number(branches[i].idBranch)));
        m_branchesTable->setItem(i, 1, new QTableWidgetItem(branches[i].branchName));
        m_branchesTable->setItem(i, 2, new QTableWidgetItem(branches[i].address));
        m_branchesTable->setItem(i, 3, new QTableWidgetItem(branches[i].phone));
    }

    if (m_branchReportsTable) {
        auto reports = DBManager::instance().getBranchReports();
        m_branchReportsTable->setRowCount(reports.size());
        for (int i = 0; i < reports.size(); ++i) {
            m_branchReportsTable->setItem(i, 0, new QTableWidgetItem(QString::number(reports[i].idBranch)));
            m_branchReportsTable->setItem(i, 1, new QTableWidgetItem(reports[i].branchName));
            m_branchReportsTable->setItem(i, 2, new QTableWidgetItem(QString::number(reports[i].servicesRendered)));
            m_branchReportsTable->setItem(i, 3, new QTableWidgetItem(QString("%1 ₽").arg(reports[i].totalRevenue, 0, 'f', 2)));
        }
    }
}

void MainWindow::loadHaircuts() {
    if (!m_haircutsTable) return;
    auto haircuts = DBManager::instance().getAllHaircuts();
    m_haircutsTable->setRowCount(haircuts.size());
    for (int i = 0; i < haircuts.size(); ++i) {
        m_haircutsTable->setItem(i, 0, new QTableWidgetItem(QString::number(haircuts[i].idHaircut)));
        m_haircutsTable->setItem(i, 1, new QTableWidgetItem(haircuts[i].haircutName));
        m_haircutsTable->setItem(i, 2, new QTableWidgetItem(haircuts[i].gender));
        m_haircutsTable->setItem(i, 3, new QTableWidgetItem(QString("%1 ₽").arg(haircuts[i].currentPrice, 0, 'f', 2)));
    }
    if (!haircuts.isEmpty()) {
        m_haircutsTable->selectRow(0);
        onHaircutSelectionChanged();
    }
}
void MainWindow::loadUsers() {
    if (!m_usersTable) return;
    auto users = DBManager::instance().getAllUsers();
    m_usersTable->setRowCount(users.size());
    for (int i = 0; i < users.size(); ++i) {
        m_usersTable->setItem(i, 0, new QTableWidgetItem(QString::number(users[i].idUser)));
        m_usersTable->setItem(i, 1, new QTableWidgetItem(users[i].login));
        QStringList roleNames;
        for (const auto &r : users[i].roles) {
            roleNames << r.roleName;
        }
        m_usersTable->setItem(i, 2, new QTableWidgetItem(roleNames.join(", ")));
    }
}

void MainWindow::loadClients() {
    if (!m_clientsTable) return;
    auto stats = DBManager::instance().getClientStats();
    m_clientsTable->setRowCount(stats.size());
    for (int i = 0; i < stats.size(); ++i) {
        m_clientsTable->setItem(i, 0, new QTableWidgetItem(QString::number(stats[i].idClient)));
        m_clientsTable->setItem(i, 1, new QTableWidgetItem(stats[i].clientName));
        m_clientsTable->setItem(i, 2, new QTableWidgetItem(stats[i].phone));
        m_clientsTable->setItem(i, 3, new QTableWidgetItem(QString::number(stats[i].totalVisits)));
        m_clientsTable->setItem(i, 4, new QTableWidgetItem(stats[i].clientStatus));
        m_clientsTable->setItem(i, 5, new QTableWidgetItem(QString("%1 ₽").arg(stats[i].totalSpent, 0, 'f', 2)));
    }
}
void MainWindow::loadVisits() {
    if (!m_visitsTable) return;

    QVector<Document> docs;
    if (m_userRole == "Master" && m_employeeBranchId > 0) {
        docs = DBManager::instance().getDocumentsByBranch(m_employeeBranchId);
    } else {
        int filterBranchId = 0;
        if (m_visitBranchFilter) {
            filterBranchId = m_visitBranchFilter->currentData().toInt();
        }
        if (filterBranchId > 0) {
            docs = DBManager::instance().getDocumentsByBranch(filterBranchId);
        } else {
            docs = DBManager::instance().getAllDocuments();
        }
    }

    m_visitsTable->setRowCount(docs.size());
    for (int i = 0; i < docs.size(); ++i) {
        m_visitsTable->setItem(i, 0, new QTableWidgetItem(QString::number(docs[i].idDocument)));
        m_visitsTable->setItem(i, 1, new QTableWidgetItem(docs[i].serviceDate.toString("dd.MM.yyyy HH:mm")));
        m_visitsTable->setItem(i, 2, new QTableWidgetItem(docs[i].branchName));
        m_visitsTable->setItem(i, 3, new QTableWidgetItem(docs[i].employeeName));
        m_visitsTable->setItem(i, 4, new QTableWidgetItem(docs[i].clientName));
        m_visitsTable->setItem(i, 5, new QTableWidgetItem(docs[i].haircutName));
        m_visitsTable->setItem(i, 6, new QTableWidgetItem(QString("%1%").arg(docs[i].discountRate, 0, 'f', 0)));
        m_visitsTable->setItem(i, 7, new QTableWidgetItem(QString("%1 ₽").arg(docs[i].finalCost, 0, 'f', 2)));
    }
}
void MainWindow::loadClientProfile() {
    if (m_clientId <= 0) {
        auto clientOpt = DBManager::instance().getClientByUserId(m_currentUser.idUser);
        if (clientOpt.has_value()) {
            m_clientId = clientOpt->idClient;
        }
    }

    if (m_clientId <= 0) return;

    auto clientOpt = DBManager::instance().getClientById(m_clientId);
    if (!clientOpt.has_value()) return;

    Client c = clientOpt.value();
    if (m_cabinetNameLabel) m_cabinetNameLabel->setText(c.fullName());
    if (m_cabinetPhoneLabel) m_cabinetPhoneLabel->setText(c.phone);

    int visits = DBManager::instance().getClientVisitCount(m_clientId);
    if (m_cabinetVisitsLabel) m_cabinetVisitsLabel->setText(QString("%1 визита(-ов)").arg(visits));

    if (m_cabinetDiscountLabel) {
        if (visits >= 5) {
            m_cabinetDiscountLabel->setText("3% — Статус: Постоянный клиент");
            m_cabinetDiscountLabel->setStyleSheet("color: #276749; font-weight: bold; border: none; font-size: 13px;");
        } else {
            m_cabinetDiscountLabel->setText(QString("0% (До получения скидки 3% осталось %1 визита(-ов))").arg(5 - visits));
            m_cabinetDiscountLabel->setStyleSheet("color: #4a5568; font-weight: bold; border: none; font-size: 13px;");
        }
    }

    double totalSpent = DBManager::instance().getClientTotalSpent(m_clientId);
    if (m_cabinetSpentLabel) m_cabinetSpentLabel->setText(QString("%1 ₽").arg(totalSpent, 0, 'f', 2));

    if (m_cabinetVisitsTable) {
        auto docs = DBManager::instance().getDocumentsByClient(m_clientId);
        m_cabinetVisitsTable->setRowCount(docs.size());
        for (int i = 0; i < docs.size(); ++i) {
            m_cabinetVisitsTable->setItem(i, 0, new QTableWidgetItem(docs[i].serviceDate.toString("dd.MM.yyyy HH:mm")));
            m_cabinetVisitsTable->setItem(i, 1, new QTableWidgetItem(docs[i].branchName));
            m_cabinetVisitsTable->setItem(i, 2, new QTableWidgetItem(docs[i].employeeName));
            m_cabinetVisitsTable->setItem(i, 3, new QTableWidgetItem(docs[i].haircutName));
            m_cabinetVisitsTable->setItem(i, 4, new QTableWidgetItem(QString("%1%").arg(docs[i].discountRate, 0, 'f', 0)));
            m_cabinetVisitsTable->setItem(i, 5, new QTableWidgetItem(QString("%1 ₽").arg(docs[i].finalCost, 0, 'f', 2)));
        }
    }
}
