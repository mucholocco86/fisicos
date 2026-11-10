#pragma once
#include <QMainWindow>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <QStringList>
class QListWidget;
class QLineEdit;
class QComboBox;
class QPushButton;
class QTextEdit;
class QDialog;
class QCheckBox;
class QSpinBox;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
private:
    QString scrcpyPath() const;
    QString adbPath() const;
    QString selectedSerial() const;
    void refreshDevices();
    void runAdb(const QStringList &args, bool refresh = false);
    void launchScrcpy(const QString &serial);
    void openSettings();
    void log(const QString &message);
    void populateDevices(const QString &output);
    QSettings settings;
    QPointer<QDialog> settingsWindow;
    QListWidget *devices = nullptr;
    QComboBox *serials = nullptr;
    QLineEdit *deviceName = nullptr;
    QLineEdit *ip = nullptr;
    QLineEdit *port = nullptr;
    QTextEdit *output = nullptr;
    QLineEdit *adbCommand = nullptr;
    QSpinBox *bitrate = nullptr;
    QSpinBox *maxSize = nullptr;
    QSpinBox *maxFps = nullptr;
    QCheckBox *stayOnTop = nullptr;
    QCheckBox *turnScreenOff = nullptr;
    QCheckBox *showFps = nullptr;
    QCheckBox *recordVideo = nullptr;
    QList<QProcess *> sessions;
};
