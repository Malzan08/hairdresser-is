#include "PasswordChangeDialog.h"
#include "LoginWindow.h"

PasswordChangeDialog::PasswordChangeDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Смена пароля");
    setFixedWidth(400);
    setupUi();
}

void PasswordChangeDialog::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Минимум 8 симв., цифра, буква, спецсимвол");
    formLayout->addRow("Новый пароль*:", m_passwordEdit);

    m_confirmEdit = new QLineEdit(this);
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    m_confirmEdit->setPlaceholderText("Повторите новый пароль");
    formLayout->addRow("Повтор пароля*:", m_confirmEdit);

    layout->addLayout(formLayout);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #e53e3e; font-size: 11px;");
    m_errorLabel->setWordWrap(true);
    layout->addWidget(m_errorLabel);

    auto *btnLayout = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("Отмена", this);
    auto *saveBtn = new QPushButton("Сохранить", this);
    saveBtn->setStyleSheet("background-color: #3182ce; color: white; font-weight: bold; padding: 6px 14px;");

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &PasswordChangeDialog::onSaveClicked);

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);

    layout->addLayout(btnLayout);
}

QString PasswordChangeDialog::newPassword() const {
    return m_passwordEdit->text();
}

void PasswordChangeDialog::onSaveClicked() {
    QString pwd = m_passwordEdit->text();
    QString confirm = m_confirmEdit->text();

    if (pwd.isEmpty() || confirm.isEmpty()) {
        m_errorLabel->setText("Заполните оба поля пароля.");
        return;
    }

    if (pwd != confirm) {
        m_errorLabel->setText("Пароли не совпадают.");
        return;
    }

    QString err;
    if (!LoginWindow::validatePassword(pwd, err)) {
        m_errorLabel->setText(err);
        return;
    }

    accept();
}
