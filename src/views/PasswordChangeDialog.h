#ifndef PASSWORDCHANGEDIALOG_H
#define PASSWORDCHANGEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

class PasswordChangeDialog : public QDialog {
    Q_OBJECT

public:
    explicit PasswordChangeDialog(QWidget *parent = nullptr);

    QString newPassword() const;

private slots:
    void onSaveClicked();

private:
    void setupUi();

    QLineEdit *m_passwordEdit{nullptr};
    QLineEdit *m_confirmEdit{nullptr};
    QLabel *m_errorLabel{nullptr};
};

#endif
