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
    layout->setContentsMargins(15, 15, 15, 15);
    layout->setSpacing(8);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(8);

    m_regLastNameEdit = new QLineEdit(tab);
    m_regLastNameEdit->setPlaceholderText("Иванов");
    formLayout->addRow("Фамилия*:", m_regLastNameEdit);

    m_regFirstNameEdit = new QLineEdit(tab);
    m_regFirstNameEdit->setPlaceholderText("Иван");
    formLayout->addRow("Имя*:", m_regFirstNameEdit);

    m_regMiddleNameEdit = new QLineEdit(tab);
    m_regMiddleNameEdit->setPlaceholderText("Иванович (необязательно)");
    formLayout->addRow("Отчество:", m_regMiddleNameEdit);

    m_regPhoneEdit = new QLineEdit(tab);
    m_regPhoneEdit->setPlaceholderText("+7(999)000-00-00");
    formLayout->addRow("Телефон*:", m_regPhoneEdit);

    m_regLoginEdit = new QLineEdit(tab);
    m_regLoginEdit->setPlaceholderText("Логин");
    formLayout->addRow("Логин*:", m_regLoginEdit);

    m_regPasswordEdit = new QLineEdit(tab);
    m_regPasswordEdit->setEchoMode(QLineEdit::Password);
    m_regPasswordEdit->setPlaceholderText("Минимум 8 симв., цифра, буква, спецсимвол");
    formLayout->addRow("Пароль*:", m_regPasswordEdit);

    m_regPasswordConfirmEdit = new QLineEdit(tab);
    m_regPasswordConfirmEdit->setEchoMode(QLineEdit::Password);
    m_regPasswordConfirmEdit->setPlaceholderText("Повторите пароль");
    formLayout->addRow("Повтор*:", m_regPasswordConfirmEdit);

    layout->addLayout(formLayout);

    m_regStatusLabel = new QLabel(tab);
    m_regStatusLabel->setWordWrap(true);
    m_regStatusLabel->setStyleSheet("color: #e53e3e; font-size: 11px;");
    layout->addWidget(m_regStatusLabel);

    layout->addStretch();

    auto *regBtn = new QPushButton("Зарегистрироваться", tab);
    regBtn->setFixedHeight(36);
    regBtn->setStyleSheet("background-color: #38a169; color: white; font-weight: bold; border-radius: 4px;");
    connect(regBtn, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    layout->addWidget(regBtn);

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
    QString lastName = m_regLastNameEdit->text().trimmed();
    QString firstName = m_regFirstNameEdit->text().trimmed();
    QString middleName = m_regMiddleNameEdit->text().trimmed();
    QString phone = m_regPhoneEdit->text().trimmed();
    QString login = m_regLoginEdit->text().trimmed();
    QString password = m_regPasswordEdit->text();
    QString confirm = m_regPasswordConfirmEdit->text();

    if (lastName.isEmpty() || firstName.isEmpty() || phone.isEmpty() || login.isEmpty() || password.isEmpty()) {
        m_regStatusLabel->setText("Пожалуйста, заполните все обязательные поля (*).");
        return;
    }

    if (password != confirm) {
        m_regStatusLabel->setText("Введенные пароли не совпадают.");
        return;
    }

    QString pwdError;
    if (!validatePassword(password, pwdError)) {
        m_regStatusLabel->setText(pwdError);
        return;
    }

    int newUserId = 0;
    if (!DBManager::instance().registerUser(login, password, "Client", &newUserId)) {
        m_regStatusLabel->setText(DBManager::instance().lastError());
        return;
    }

    if (!DBManager::instance().addClient(lastName, firstName, middleName, phone, newUserId)) {
        m_regStatusLabel->setText(DBManager::instance().lastError());
        return;
    }

    QMessageBox::information(this, "Успех", "Регистрация прошла успешно! Теперь вы можете войти в систему.");
    m_tabWidget->setCurrentIndex(0);
    m_loginEdit->setText(login);
    m_passwordEdit->setText(password);
    m_regStatusLabel->clear();
}

bool LoginWindow::validatePassword(const QString &password, QString &outError) {
    if (password.length() < 8) {
        outError = "Пароль должен быть длиной не менее 8 символов.";
        return false;
    }
    QRegularExpression digitRegex("[0-9]");
    if (!digitRegex.match(password).hasMatch()) {
        outError = "Пароль должен содержать хотя бы одну цифру (0-9).";
        return false;
    }
    QRegularExpression upperRegex("[A-ZА-ЯЁ]");
    if (!upperRegex.match(password).hasMatch()) {
        outError = "Пароль должен содержать хотя бы одну заглавную букву (A-Z или А-Я).";
        return false;
    }
    QRegularExpression specialRegex("[^a-zA-Z0-9а-яА-ЯёЁ\\s]");
    if (!specialRegex.match(password).hasMatch()) {
        outError = "Пароль должен содержать хотя бы один специальный символ (!@#$%^&* и др.).";
        return false;
    }
    outError.clear();
    return true;
}
