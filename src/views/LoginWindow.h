#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include "../models/User.h"

class LoginWindow : public QDialog {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow() override = default;

    User authenticatedUser() const;
    static bool validatePassword(const QString &password, QString &outError);

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void setupUi();
    QWidget* createLoginTab();
    QWidget* createRegisterTab();

    QTabWidget *m_tabWidget{nullptr};

    QLineEdit *m_loginEdit{nullptr};
    QLineEdit *m_passwordEdit{nullptr};
    QLabel *m_statusLabel{nullptr};

    QLineEdit *m_regLastNameEdit{nullptr};
    QLineEdit *m_regFirstNameEdit{nullptr};
    QLineEdit *m_regMiddleNameEdit{nullptr};
    QLineEdit *m_regPhoneEdit{nullptr};
    QLineEdit *m_regLoginEdit{nullptr};
    QLineEdit *m_regPasswordEdit{nullptr};
    QLineEdit *m_regPasswordConfirmEdit{nullptr};
    QLabel *m_regStatusLabel{nullptr};

    User m_authenticatedUser;
};

#endif
