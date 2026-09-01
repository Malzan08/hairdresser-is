#ifndef VISITDIALOG_H
#define VISITDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include "../models/Branch.h"
#include "../models/Employee.h"
#include "../models/Client.h"
#include "../models/Haircut.h"

class VisitDialog : public QDialog {
    Q_OBJECT

public:
    explicit VisitDialog(int defaultBranchId = 0, int defaultEmployeeId = 0, QWidget *parent = nullptr);

    int selectedBranchId() const;
    int selectedEmployeeId() const;
    int selectedClientId() const;
    int selectedHaircutId() const;
    QDateTime selectedDateTime() const;

private slots:
    void onSearchClientByPhone();
    void onHaircutChanged(int index);
    void onClientChanged(int index);
    void onBranchChanged(int index);
    void onSaveClicked();

private:
    void setupUi();
    void loadData();
    void recalculateCost();

    int m_defaultBranchId{0};
    int m_defaultEmployeeId{0};

    QComboBox *m_branchCombo{nullptr};
    QComboBox *m_employeeCombo{nullptr};
    QComboBox *m_clientCombo{nullptr};
    QLineEdit *m_phoneSearchEdit{nullptr};
    QPushButton *m_searchPhoneBtn{nullptr};

    QComboBox *m_haircutCombo{nullptr};
    QDateTimeEdit *m_dateTimeEdit{nullptr};

    QLabel *m_basePriceLabel{nullptr};
    QLabel *m_discountLabel{nullptr};
    QLabel *m_finalCostLabel{nullptr};
    QLabel *m_errorLabel{nullptr};

    QVector<Branch> m_branches;
    QVector<Employee> m_employees;
    QVector<Client> m_clients;
    QVector<Haircut> m_haircuts;
};

#endif
