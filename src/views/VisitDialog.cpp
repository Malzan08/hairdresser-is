#include "VisitDialog.h"
#include "../controllers/DBManager.h"

VisitDialog::VisitDialog(int defaultBranchId, int defaultEmployeeId, QWidget *parent)
    : QDialog(parent), m_defaultBranchId(defaultBranchId), m_defaultEmployeeId(defaultEmployeeId) {
    setWindowTitle("Оформление визита клиента");
    setFixedWidth(460);
    setupUi();
    loadData();
    recalculateCost();
}

void VisitDialog::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_branchCombo = new QComboBox(this);
    formLayout->addRow("Филиал:", m_branchCombo);
    connect(m_branchCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VisitDialog::onBranchChanged);

    m_employeeCombo = new QComboBox(this);
    formLayout->addRow("Мастер:", m_employeeCombo);

    auto *clientSearchLayout = new QHBoxLayout();
    m_phoneSearchEdit = new QLineEdit(this);
    m_phoneSearchEdit->setPlaceholderText("Поиск по тел...");
    m_searchPhoneBtn = new QPushButton("Найти", this);
    clientSearchLayout->addWidget(m_phoneSearchEdit);
    clientSearchLayout->addWidget(m_searchPhoneBtn);
    connect(m_searchPhoneBtn, &QPushButton::clicked, this, &VisitDialog::onSearchClientByPhone);
    formLayout->addRow("Быстрый поиск:", clientSearchLayout);

    m_clientCombo = new QComboBox(this);
    formLayout->addRow("Клиент*:", m_clientCombo);
    connect(m_clientCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VisitDialog::onClientChanged);

    m_haircutCombo = new QComboBox(this);
    formLayout->addRow("Стрижка / Услуга*:", m_haircutCombo);
    connect(m_haircutCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VisitDialog::onHaircutChanged);

    m_dateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_dateTimeEdit->setDisplayFormat("dd.MM.yyyy HH:mm");
    m_dateTimeEdit->setCalendarPopup(true);
    formLayout->addRow("Дата и время*:", m_dateTimeEdit);

    m_basePriceLabel = new QLabel("0.00 ₽", this);
    m_basePriceLabel->setStyleSheet("font-weight: bold;");
    formLayout->addRow("Базовая цена:", m_basePriceLabel);

    m_discountLabel = new QLabel("0% (Обычный клиент)", this);
    m_discountLabel->setStyleSheet("color: #2b6cb0; font-weight: bold;");
    formLayout->addRow("Скидка:", m_discountLabel);

    m_finalCostLabel = new QLabel("0.00 ₽", this);
    QFont f = m_finalCostLabel->font();
    f.setPointSize(12);
    f.setBold(true);
    m_finalCostLabel->setFont(f);
    m_finalCostLabel->setStyleSheet("color: #22543d;");
    formLayout->addRow("К оплате:", m_finalCostLabel);

    layout->addLayout(formLayout);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #e53e3e; font-size: 11px;");
    m_errorLabel->setWordWrap(true);
    layout->addWidget(m_errorLabel);

    auto *btnLayout = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("Отмена", this);
    auto *saveBtn = new QPushButton("Подтвердить визит", this);
    saveBtn->setStyleSheet("background-color: #38a169; color: white; font-weight: bold; padding: 6px 14px;");

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &VisitDialog::onSaveClicked);

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);

    layout->addLayout(btnLayout);
}

void VisitDialog::loadData() {
    m_branches = DBManager::instance().getAllBranches();
    m_branchCombo->clear();
    int defaultBranchIdx = 0;
    for (int i = 0; i < m_branches.size(); ++i) {
        m_branchCombo->addItem(m_branches[i].branchName, m_branches[i].idBranch);
        if (m_branches[i].idBranch == m_defaultBranchId) {
            defaultBranchIdx = i;
        }
    }
    if (!m_branches.isEmpty()) {
        m_branchCombo->setCurrentIndex(defaultBranchIdx);
    }
    onBranchChanged(defaultBranchIdx);

    m_clients = DBManager::instance().getAllClients();
    m_clientCombo->clear();
    for (const auto &c : m_clients) {
        m_clientCombo->addItem(QString("%1 (%2)").arg(c.fullName(), c.phone), c.idClient);
    }

    m_haircuts = DBManager::instance().getAllHaircuts();
    m_haircutCombo->clear();
    for (const auto &h : m_haircuts) {
        m_haircutCombo->addItem(QString("%1 (%2) — %3 ₽").arg(h.haircutName, h.gender).arg(h.currentPrice, 0, 'f', 2), h.idHaircut);
    }

    if (m_defaultBranchId > 0) {
        m_branchCombo->setEnabled(false);
    }
}

void VisitDialog::onBranchChanged(int index) {
    if (index < 0 || index >= m_branches.size()) return;
    int branchId = m_branches[index].idBranch;
    m_employees = DBManager::instance().getEmployeesByBranch(branchId);
    m_employeeCombo->clear();
    int defaultEmpIdx = 0;
    for (int i = 0; i < m_employees.size(); ++i) {
        m_employeeCombo->addItem(QString("%1 %2 (%3)").arg(m_employees[i].lastName, m_employees[i].firstName, m_employees[i].position), m_employees[i].idEmployee);
        if (m_employees[i].idEmployee == m_defaultEmployeeId) {
            defaultEmpIdx = i;
        }
    }
    if (!m_employees.isEmpty()) {
        m_employeeCombo->setCurrentIndex(defaultEmpIdx);
    }
    if (m_defaultEmployeeId > 0) {
        m_employeeCombo->setEnabled(false);
    }
}

void VisitDialog::onHaircutChanged(int) {
    recalculateCost();
}

void VisitDialog::onClientChanged(int) {
    recalculateCost();
}

void VisitDialog::onSearchClientByPhone() {
    QString phone = m_phoneSearchEdit->text().trimmed();
    if (phone.isEmpty()) return;

    auto clientOpt = DBManager::instance().getClientByPhone(phone);
    if (!clientOpt.has_value()) {
        m_errorLabel->setText("Клиент с таким телефоном не найден.");
        return;
    }

    m_errorLabel->clear();
    for (int i = 0; i < m_clientCombo->count(); ++i) {
        if (m_clientCombo->itemData(i).toInt() == clientOpt->idClient) {
            m_clientCombo->setCurrentIndex(i);
            break;
        }
    }
}

void VisitDialog::recalculateCost() {
    int haircutIdx = m_haircutCombo->currentIndex();
    if (haircutIdx < 0 || haircutIdx >= m_haircuts.size()) {
        m_basePriceLabel->setText("0.00 ₽");
        m_finalCostLabel->setText("0.00 ₽");
        return;
    }

    double basePrice = m_haircuts[haircutIdx].currentPrice;
    m_basePriceLabel->setText(QString("%1 ₽").arg(basePrice, 0, 'f', 2));

    int clientIdx = m_clientCombo->currentIndex();
    double discountRate = 0.0;
    if (clientIdx >= 0 && clientIdx < m_clients.size()) {
        int clientId = m_clients[clientIdx].idClient;
        int visits = DBManager::instance().getClientVisitCount(clientId);
        if (visits >= 4) {
            discountRate = 3.0;
            m_discountLabel->setText(QString("3% (Постоянный клиент, визитов: %1)").arg(visits));
            m_discountLabel->setStyleSheet("color: #276749; font-weight: bold;");
        } else {
            discountRate = 0.0;
            m_discountLabel->setText(QString("0% (До скидки осталось %1 визита(-ов))").arg(4 - visits));
            m_discountLabel->setStyleSheet("color: #4a5568; font-weight: bold;");
        }
    }

    double finalCost = basePrice * (1.0 - (discountRate / 100.0));
    m_finalCostLabel->setText(QString("%1 ₽").arg(finalCost, 0, 'f', 2));
}

int VisitDialog::selectedBranchId() const {
    return m_branchCombo->currentData().toInt();
}

int VisitDialog::selectedEmployeeId() const {
    return m_employeeCombo->currentData().toInt();
}

int VisitDialog::selectedClientId() const {
    return m_clientCombo->currentData().toInt();
}

int VisitDialog::selectedHaircutId() const {
    return m_haircutCombo->currentData().toInt();
}

QDateTime VisitDialog::selectedDateTime() const {
    return m_dateTimeEdit->dateTime();
}

void VisitDialog::onSaveClicked() {
    if (selectedBranchId() <= 0 || selectedEmployeeId() <= 0 || selectedClientId() <= 0 || selectedHaircutId() <= 0) {
        m_errorLabel->setText("Пожалуйста, убедитесь, что выбраны все поля визита.");
        return;
    }
    accept();
}
