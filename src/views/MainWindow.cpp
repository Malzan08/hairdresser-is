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
void MainWindow::loadUsers() {}
void MainWindow::loadClients() {}
void MainWindow::loadVisits() {}
void MainWindow::loadClientProfile() {}
