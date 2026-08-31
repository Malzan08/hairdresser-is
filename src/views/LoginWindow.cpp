#include "LoginWindow.h"
#include "../controllers/DBManager.h"

LoginWindow::LoginWindow(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Авторизация в системе - Парикмахерская");
    setFixedSize(480, 520);
    setupUi();
}

User LoginWindow::authenticatedUser() const {
    return m_authenticatedUser;
}

void LoginWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    auto *headerLabel = new QLabel("Информационная система\n«Парикмахерская»", this);
    headerLabel->setAlignment(Qt::AlignCenter);
    QFont hFont = headerLabel->font();
    hFont.setPointSize(14);
    hFont.setBold(true);
    headerLabel->setFont(hFont);
    headerLabel->setStyleSheet("color: #1a365d; margin-bottom: 10px;");
    mainLayout->addWidget(headerLabel);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->addTab(createLoginTab(), "Вход в систему");
    m_tabWidget->addTab(createRegisterTab(), "Регистрация");
    mainLayout->addWidget(m_tabWidget);
}

QWidget* LoginWindow::createLoginTab() {
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);
    layout->setContentsMargins(15, 20, 15, 15);
    layout->setSpacing(12);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_loginEdit = new QLineEdit(tab);
    m_loginEdit->setPlaceholderText("Введите логин");
    formLayout->addRow("Логин:", m_loginEdit);

    m_passwordEdit = new QLineEdit(tab);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Введите пароль");
    formLayout->addRow("Пароль:", m_passwordEdit);

    layout->addLayout(formLayout);

    m_statusLabel = new QLabel(tab);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("color: #e53e3e; font-size: 11px;");
    layout->addWidget(m_statusLabel);

    layout->addStretch();

    auto *loginBtn = new QPushButton("Войти", tab);
    loginBtn->setFixedHeight(36);
    loginBtn->setStyleSheet("background-color: #3182ce; color: white; font-weight: bold; border-radius: 4px;");
    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    layout->addWidget(loginBtn);

    auto *exitBtn = new QPushButton("Выход", tab);
    exitBtn->setFixedHeight(32);
    connect(exitBtn, &QPushButton::clicked, this, &QDialog::reject);
    layout->addWidget(exitBtn);

    return tab;
}

QWidget* LoginWindow::createRegisterTab() {
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);
    m_regStatusLabel = new QLabel("Форма в разработке", tab);
    layout->addWidget(m_regStatusLabel);
    return tab;
}

void LoginWindow::onLoginClicked() {
    QString login = m_loginEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        m_statusLabel->setText("Пожалуйста, заполните логин и пароль.");
        return;
    }

    auto userOpt = DBManager::instance().authenticateUser(login, password);
    if (!userOpt.has_value()) {
        m_statusLabel->setText(DBManager::instance().lastError());
        return;
    }

    m_authenticatedUser = userOpt.value();
    m_statusLabel->clear();
    accept();
}

void LoginWindow::onRegisterClicked() {
}

bool LoginWindow::validatePassword(const QString &password, QString &outError) {
    if (password.length() < 8) {
        outError = "Пароль должен содержать не менее 8 символов.";
        return false;
    }
    return true;
}
