#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include "../models/Client.h"

class ClientDialog : public QDialog {
    Q_OBJECT

public:
    explicit ClientDialog(QWidget *parent = nullptr);
    explicit ClientDialog(const Client &client, QWidget *parent = nullptr);

    QString lastName() const;
    QString firstName() const;
    QString middleName() const;
    QString phone() const;

private slots:
    void onSaveClicked();

private:
    void setupUi();

    QLineEdit *m_lastNameEdit{nullptr};
    QLineEdit *m_firstNameEdit{nullptr};
    QLineEdit *m_middleNameEdit{nullptr};
    QLineEdit *m_phoneEdit{nullptr};
    QLabel *m_errorLabel{nullptr};

    bool m_isEditMode{false};
    int m_clientId{0};
};

#endif
