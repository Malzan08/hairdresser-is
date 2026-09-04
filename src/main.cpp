#include <QApplication>
#include <QMessageBox>
#include <QStyleFactory>
#include "controllers/DBManager.h"
#include "views/LoginWindow.h"
#include "views/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("HairdresserIS");
    app.setApplicationDisplayName("ИС «Парикмахерская»");
    app.setStyle(QStyleFactory::create("Fusion"));

    QString qss = "QWidget { font-family: 'Segoe UI', Arial, sans-serif; font-size: 13px; }"
                  "QHeaderView::section { background-color: #edf2f7; padding: 4px; border: 1px solid #e2e8f0; font-weight: bold; }"
                  "QTableWidget { gridline-color: #edf2f7; selection-background-color: #bee3f8; selection-color: #2b6cb0; }"
                  "QTabWidget::pane { border: 1px solid #cbd5e0; background: white; }"
                  "QTabBar::tab { background: #e2e8f0; padding: 8px 16px; margin-right: 2px; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
                  "QTabBar::tab:selected { background: white; font-weight: bold; color: #2b6cb0; }"
                  "QPushButton { border-radius: 4px; padding: 5px 12px; }"
                  "QLineEdit, QComboBox, QDoubleSpinBox, QDateTimeEdit { padding: 4px 6px; border: 1px solid #cbd5e0; border-radius: 4px; }"
                  "QLineEdit:focus, QComboBox:focus, QDoubleSpinBox:focus { border: 1px solid #3182ce; }";
    app.setStyleSheet(qss);

    if (!DBManager::instance().connectDatabase()) {
        QMessageBox::critical(nullptr, "Критическая ошибка",
                              "Не удалось подключиться к базе данных PostgreSQL:\n" + DBManager::instance().lastError());
        return 1;
    }

    bool appRunning = true;
    while (appRunning) {
        LoginWindow loginWin;
        if (loginWin.exec() != QDialog::Accepted) {
            appRunning = false;
            break;
        }

        User currentUser = loginWin.authenticatedUser();
        MainWindow mainWin(currentUser);

        bool relogin = false;
        QObject::connect(&mainWin, &MainWindow::logoutRequested, [&]() {
            relogin = true;
            mainWin.close();
        });

        mainWin.show();
        app.exec();

        if (!relogin) {
            appRunning = false;
        }
    }

    DBManager::instance().disconnectDatabase();
    return 0;
}
