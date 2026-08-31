#ifndef BRANCHDIALOG_H
#define BRANCHDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include "../models/Branch.h"

class BranchDialog : public QDialog {
    Q_OBJECT

public:
    explicit BranchDialog(QWidget *parent = nullptr);
    explicit BranchDialog(const Branch &branch, QWidget *parent = nullptr);

    QString branchName() const;
    QString address() const;
    QString phone() const;

private slots:
    void onSaveClicked();

private:
    void setupUi();

    QLineEdit *m_nameEdit{nullptr};
    QLineEdit *m_addressEdit{nullptr};
    QLineEdit *m_phoneEdit{nullptr};
    QLabel *m_errorLabel{nullptr};

    bool m_isEditMode{false};
    int m_branchId{0};
};

#endif
