#include "HaircutDialog.h"

HaircutDialog::HaircutDialog(QWidget *parent)
    : QDialog(parent), m_isEditMode(false) {
    setWindowTitle("Новая стрижка");
    setupUi();
}

HaircutDialog::HaircutDialog(const Haircut &haircut, QWidget *parent)
    : QDialog(parent), m_isEditMode(true), m_haircutId(haircut.idHaircut) {
    setWindowTitle("Редактирование стрижки");
    setupUi();
    m_nameEdit->setText(haircut.haircutName);
    int idx = m_genderCombo->findText(haircut.gender);
    if (idx >= 0) {
        m_genderCombo->setCurrentIndex(idx);
    }
    m_priceSpin->setValue(haircut.currentPrice);
    m_priceSpin->setEnabled(false);
}

void HaircutDialog::setupUi() {
    setFixedWidth(400);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto *formLayout = new QFormLayout();
    formLayout->setSpacing(10);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Название услуги");
    formLayout->addRow("Название*:", m_nameEdit);

    m_genderCombo = new QComboBox(this);
    m_genderCombo->addItems({"Мужская", "Женская", "Унисекс"});
    formLayout->addRow("Категория*:", m_genderCombo);

    m_priceSpin = new QDoubleSpinBox(this);
    m_priceSpin->setRange(100.0, 50000.0);
    m_priceSpin->setSingleStep(50.0);
    m_priceSpin->setValue(1000.0);
    m_priceSpin->setSuffix(" ₽");
    formLayout->addRow("Цена (руб.)*:", m_priceSpin);

    layout->addLayout(formLayout);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #e53e3e; font-size: 11px;");
    m_errorLabel->setWordWrap(true);
    layout->addWidget(m_errorLabel);

    auto *btnLayout = new QHBoxLayout();
    auto *cancelBtn = new QPushButton("Отмена", this);
    auto *saveBtn = new QPushButton("Сохранить", this);
    saveBtn->setStyleSheet("background-color: #3182ce; color: white; font-weight: bold; padding: 6px 12px;");

    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, this, &HaircutDialog::onSaveClicked);

    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);

    layout->addLayout(btnLayout);
}

QString HaircutDialog::haircutName() const {
    return m_nameEdit->text().trimmed();
}

QString HaircutDialog::gender() const {
    return m_genderCombo->currentText();
}

double HaircutDialog::price() const {
    return m_priceSpin->value();
}

void HaircutDialog::onSaveClicked() {
    if (haircutName().isEmpty()) {
        m_errorLabel->setText("Введите название стрижки.");
        return;
    }
    if (price() <= 0.0) {
        m_errorLabel->setText("Цена должна быть больше нуля.");
        return;
    }
    accept();
}
