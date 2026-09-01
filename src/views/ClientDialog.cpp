#include "ClientDialog.h"

ClientDialog::ClientDialog(QWidget *parent)
    : QDialog(parent), m_isEditMode(false) {
    setWindowTitle("Новый клиент");
    setupUi();
}

ClientDialog::ClientDialog(const Client &client, QWidget *parent)
    : QDialog(parent), m_isEditMode(true), m_clientId(client.idClient) {
    setWindowTitle("Редактирование клиента");
    setupUi();
    m_lastNameEdit->setText(client.lastName);
    m_firstNameEdit->setText(client.firstName);
    m_middleNameEdit->setText(client.middleName);
    m_phoneEdit->setText(client.phone);
}

void ClientDialog::setupUi() {
    setFixedWidth(400);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_lastNameEdit = new QLineEdit(this);
    m_lastNameEdit->setPlaceholderText("Иванов");
    formLayout->addRow("Фамилия*:", m_lastNameEdit);

    m_firstNameEdit = new QLineEdit(this);
    m_firstNameEdit->setPlaceholderText("Иван");
    formLayout->addRow("Имя*:", m_firstNameEdit);

    m_middleNameEdit = new QLineEdit(this);
    m_middleNameEdit->setPlaceholderText("Иванович (необязательно)");
    formLayout->addRow("Отчество:", m_middleNameEdit);

    m_phoneEdit = new QLineEdit(this);
    m_phoneEdit->setPlaceholderText("+7(999)123-45-67");
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
    connect(saveBtn, &QPushButton::clicked, this, &ClientDialog::onSaveClicked);

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);

    layout->addLayout(btnLayout);
}

QString ClientDialog::lastName() const {
    return m_lastNameEdit->text().trimmed();
}

QString ClientDialog::firstName() const {
    return m_firstNameEdit->text().trimmed();
}

QString ClientDialog::middleName() const {
    return m_middleNameEdit->text().trimmed();
}

QString ClientDialog::phone() const {
    return m_phoneEdit->text().trimmed();
}

void ClientDialog::onSaveClicked() {
    if (lastName().isEmpty() || firstName().isEmpty() || phone().isEmpty()) {
        m_errorLabel->setText("Заполните обязательные поля: Фамилия, Имя, Телефон.");
        return;
    }
    accept();
}
