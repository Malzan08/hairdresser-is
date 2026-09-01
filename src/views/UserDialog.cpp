#include "UserDialog.h"
#include "LoginWindow.h"
#include "../controllers/DBManager.h"

UserDialog::UserDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Новый пользователь");
    setFixedWidth(400);
    setupUi();
}

void UserDialog::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_loginEdit = new QLineEdit(this);
    m_loginEdit->setPlaceholderText("Логин");
    formLayout->addRow("Логин*:", m_loginEdit);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Минимум 8 симв., цифра, буква, спецсимвол");
    formLayout->addRow("Пароль*:", m_passwordEdit);

    m_roleCombo = new QComboBox(this);
    auto roles = DBManager::instance().getAllRoles();
    for (const auto &r : roles) {
        m_roleCombo->addItem(r.roleName, r.idRole);
    }
    formLayout->addRow("Роль*:", m_roleCombo);

    layout->addLayout(formLayout);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #e53e3e; font-size: 11px;");
    m_errorLabel->setWordWrap(true);
    layout->addWidget(m_errorLabel);

    auto *btnLayout = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("Отмена", this);
    auto *saveBtn = new QPushButton("Создать", this);
    saveBtn->setStyleSheet("background-color: #3182ce; color: white; font-weight: bold; padding: 6px 14px;");

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &UserDialog::onSaveClicked);

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);

    layout->addLayout(btnLayout);
}

QString UserDialog::login() const {
    return m_loginEdit->text().trimmed();
}

QString UserDialog::password() const {
    return m_passwordEdit->text();
}

QString UserDialog::roleName() const {
    return m_roleCombo->currentText();
}

void UserDialog::onSaveClicked() {
    if (login().isEmpty() || password().isEmpty()) {
        m_errorLabel->setText("Заполните все поля.");
        return;
    }

    QString pwdErr;
    if (!LoginWindow::validatePassword(password(), pwdErr)) {
        m_errorLabel->setText(pwdErr);
        return;
    }

    if (DBManager::instance().checkLoginExists(login())) {
        m_errorLabel->setText("Пользователь с таким логином уже существует.");
        return;
    }

    accept();
}
