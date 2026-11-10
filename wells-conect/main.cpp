#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QStyleFactory>
int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("WELLS CONECT");
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setStyleSheet(R"QSS(
        QWidget { background-color:#101014; color:#f1f1f5; font-size:12px; }
        QMainWindow, QDialog { background-color:#101014; }
        QGroupBox { border:1px solid #323238; border-radius:5px; margin-top:12px; padding:10px 8px 8px; font-weight:bold; }
        QGroupBox::title { subcontrol-origin:margin; left:8px; padding:0 5px; }
        QPushButton { background-color:#c90015; border:1px solid #d71e2d; border-radius:6px; padding:5px 11px; min-height:22px; }
        QPushButton:hover { background:#e3192c; }
        QPushButton:pressed { background:#9f0719; }
        QPushButton#settingsButton { background:#29292f; border-color:#45454b; font-size:17px; min-width:42px; }
        QLineEdit, QComboBox, QSpinBox, QTextEdit, QListWidget { background:#222228; border:1px solid #404048; border-radius:3px; padding:4px; }
        QListWidget::item:selected { background:#c90015; }
        QCheckBox { spacing:6px; }
        QTabWidget::pane { border:1px solid #33333b; }
        QTabBar::tab { background:#08080b; padding:7px 12px; }
        QTabBar::tab:selected { border-bottom:2px solid #c90015; }
        QLabel#brand { font-size:27px; font-weight:800; color:#ef3a42; }
        QLabel#settingsBrand { font-size:19px; font-weight:800; color:#ef3a42; }
    )QSS");
    MainWindow w;
    w.show();
    return app.exec();
}
