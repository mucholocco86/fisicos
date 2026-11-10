#ifndef TOOLFORM_H
#define TOOLFORM_H

#include <QPointer>
#include <QWidget>
#include <QTimer>
#include <QByteArray>

#include "../QtScrcpyCore/include/QtScrcpyCore.h"
#include "magneticwidget.h"

namespace Ui
{
    class ToolForm;
}

class Device;
class QPushButton;
class ToolForm : public MagneticWidget
{
    Q_OBJECT

public:
    explicit ToolForm(QWidget *adsorbWidget, AdsorbPositions adsorbPos);
    ~ToolForm();

    void setSerial(const QString& serial);
    bool isHost();

signals:
    // Pontos de integração para etapas futuras; sem mecanismo provisório.
    void gamepadRequested(const QString &serial);
    void physicalKeyboardRequested(const QString &serial);


protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private slots:
    void on_fullScreenBtn_clicked();
    void on_returnBtn_clicked();
    void on_homeBtn_clicked();
    void on_menuBtn_clicked();
    void on_appSwitchBtn_clicked();
    void on_powerBtn_clicked();
    void on_screenShotBtn_clicked();
    void on_volumeUpBtn_clicked();
    void on_volumeDownBtn_clicked();
    void on_closeScreenBtn_clicked();
    void on_expandNotifyBtn_clicked();
    void on_expandSettingsBtn_clicked();
    void on_rotateBtn_clicked();
    void on_touchBtn_clicked();
    void on_cameraTorchBtn_clicked();
    void on_cameraZoomOutBtn_clicked();
    void on_cameraZoomInBtn_clicked();
    void on_groupControlBtn_clicked();
    void on_openScreenBtn_clicked();
    void on_clipboardBtn_clicked();

private:
    void initStyle();
    void updateGroupControl();
    void updateCameraMode();
    void setupRemodeledToolbar();
    void toggleGamepad();
    void pollGamepad();
    void togglePhysicalKeyboard();
    void stopNativeInput();
    bool sendHid(const QByteArray &packet);

private:
    Ui::ToolForm *ui;
    QPushButton *m_gamepadBtn = nullptr;
    QPushButton *m_keyboardBtn = nullptr;
    QPushButton *m_muteBtn = nullptr;
    QPoint m_dragPosition;
    QString m_serial;
    bool m_showTouch = false;
    bool m_cameraTorch = false;
    bool m_isHost = false;
    bool m_gamepadEnabled = false;
    bool m_gamepadRegistered = false;
    bool m_keyboardEnabled = false;
    QByteArray m_lastGamepadReport;
    QTimer m_gamepadTimer;
};

#endif // TOOLFORM_H
