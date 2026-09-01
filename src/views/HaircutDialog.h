#ifndef HAIRCUTDIALOG_H
#define HAIRCUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include "../models/Haircut.h"

class HaircutDialog : public QDialog {
    Q_OBJECT

public:
    explicit HaircutDialog(QWidget *parent = nullptr);
    explicit HaircutDialog(const Haircut &haircut, QWidget *parent = nullptr);

    QString haircutName() const;
    QString gender() const;
    double price() const;

private slots:
    void onSaveClicked();

private:
    void setupUi();

    QLineEdit *m_nameEdit{nullptr};
    QComboBox *m_genderCombo{nullptr};
    QDoubleSpinBox *m_priceSpin{nullptr};
    QLabel *m_errorLabel{nullptr};

    bool m_isEditMode{false};
    int m_haircutId{0};
};

#endif
