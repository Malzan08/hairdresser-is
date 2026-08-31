#include "BranchDialog.h"

BranchDialog::BranchDialog(QWidget *parent)
    : QDialog(parent), m_isEditMode(false) {
    setWindowTitle("Новый филиал");
    setupUi();
}

BranchDialog::BranchDialog(const Branch &branch, QWidget *parent)
    : QDialog(parent), m_isEditMode(true), m_branchId(branch.idBranch) {
    setWindowTitle("Редактирование филиала");
    setupUi();
    m_nameEdit->setText(branch.branchName);
    m_addressEdit->setText(branch.address);
    m_phoneEdit->setText(branch.phone);
}

void BranchDialog::setupUi() {
    setFixedWidth(400);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Филиал «Западный»");
    formLayout->addRow("Название*:", m_nameEdit);

    m_addressEdit = new QLineEdit(this);
    m_addressEdit->setPlaceholderText("ул. Ленина, д. 10");
    formLayout->addRow("Адрес*:", m_addressEdit);

    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setPlaceholderText("+7(495)123-45-67");
    formLayout->addRow("Телефон*:", m_phoneEdit);

    layout->addLayout(formLayout);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #e53e3e; font-size: 11px;");
    m_errorLabel->setWordWrap(true);
    layout->addWidget(m_errorLabel);

    auto *btnLayout = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("Отмена", this);
    auto *saveBtn = new QPushButton("Сохранить", this);
    saveBtn->setStyleSheet("background-color: #3182ce; color: white; font-weight: bold; padding: 6px 12px;");

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &BranchDialog::onSaveClicked);

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);

    layout->addLayout(btnLayout);
}

QString BranchDialog::branchName() const {
    return m_nameEdit->text().trimmed();
}

QString BranchDialog::address() const {
    return m_addressEdit->text().trimmed();
}

QString BranchDialog::phone() const {
    return m_phoneEdit->text().trimmed();
}

void BranchDialog::onSaveClicked() {
    if (branchName().isEmpty() || address().isEmpty() || phone().isEmpty()) {
        m_errorLabel->setText("Заполните все обязательные поля.");
        return;
    }
    accept();
}
