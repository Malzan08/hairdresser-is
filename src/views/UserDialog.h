#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include "../models/Role.h"

class UserDialog : public QDialog {
    Q_OBJECT

public:
    explicit UserDialog(QWidget *parent = nullptr);

    QString login() const;
    QString password() const;
    QString roleName() const;

private slots:
    void onSaveClicked();

private:
    void setupUi();

    QLineEdit *m_loginEdit{nullptr};
    QLineEdit *m_passwordEdit{nullptr};
    QComboBox *m_roleCombo{nullptr};
    QLabel *m_errorLabel{nullptr};
};

#endif
