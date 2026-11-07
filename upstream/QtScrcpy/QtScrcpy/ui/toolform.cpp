#include <QDebug>
#include <QHideEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include "nativehidcontrols.h"

#include "iconhelper.h"
#include "toolform.h"
#include "ui_toolform.h"
#include "videoform.h"
#include "../groupcontroller/groupcontroller.h"

ToolForm::ToolForm(QWidget *adsorbWidget, AdsorbPositions adsorbPos) : MagneticWidget(adsorbWidget, adsorbPos), ui(new Ui::ToolForm)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    //setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint);

    updateGroupControl();

    setupRemodeledToolbar();
    initStyle();
    m_gamepadTimer.setInterval(16);
    connect(&m_gamepadTimer, &QTimer::timeout, this, &ToolForm::pollGamepad);
}

ToolForm::~ToolForm()
{
    stopNativeInput();
    delete ui;
}

void ToolForm::setSerial(const QString &serial)
{
    if (m_serial != serial) stopNativeInput();
    m_serial = serial;
    updateCameraMode();
}

bool ToolForm::isHost()
{
    return m_isHost;
}

void ToolForm::setupRemodeledToolbar()
{
    // Reorganiza somente a barra externa do vídeo; os controles originais
    // permanecem implementados, inclusive atalhos de teclado.
    auto *layout = ui->verticalLayout;
    layout->setContentsMargins(3, 5, 3, 5);
    layout->setSpacing(2);

    auto createButton = [this](const char *name, const QString &hint) {
        auto *button = new QPushButton(this);
        button->setObjectName(QString::fromLatin1(name));
        button->setToolTip(hint);
        button->setFixedSize(36, 35);
        return button;
    };

    m_keymapperBtn = createButton("keymapperBtn",
        QStringLiteral("Abrir ScrcpyKeyMapper integrado"));
    m_gamepadBtn = createButton("gamepadBtn",
        QStringLiteral("Ativar/desativar controle físico conectado ao PC"));
    m_keyboardBtn = createButton("physicalKeyboardBtn",
        QStringLiteral("Alternar teclado físico e teclado virtual do Android"));
    m_screenFitBtn = createButton("screenFitBtn",
        QStringLiteral("Ajustar o espelhamento à janela"));
    m_muteBtn = createButton("muteBtn",
        QStringLiteral("Silenciar áudio (integração em etapa específica)"));

    // O mapper já possui um sinal dedicado para a integração posterior.
    // Seus botões ficam ativos; os outros dois executam UHID no canal existente.
    m_keymapperBtn->setEnabled(true);
    m_gamepadBtn->setEnabled(true);
    m_keyboardBtn->setEnabled(true);
    m_gamepadBtn->setCheckable(true);
    m_keyboardBtn->setCheckable(true);
    m_muteBtn->setEnabled(false);

    connect(m_keymapperBtn, &QPushButton::clicked, this, [this]() {
        emit keyMapperRequested(m_serial);
    });
    connect(m_gamepadBtn, &QPushButton::clicked, this, [this]() {
        toggleGamepad();
        emit gamepadRequested(m_serial);
    });
    connect(m_keyboardBtn, &QPushButton::clicked, this, [this]() {
        togglePhysicalKeyboard();
        emit physicalKeyboardRequested(m_serial);
    });

    // Os dois ajustes já existem em VideoForm e dispensam módulos novos.
    connect(m_screenFitBtn, &QPushButton::clicked, this, [this]() {
        auto *video = qobject_cast<VideoForm *>(parentWidget());
        if (video) video->removeBlackRect();
    });

    // Ordem visual da imagem de referência: grupo, tela cheia,
    // mapeador, gamepad, teclado, enquadramento, áudio e navegação.
    const QList<QPushButton *> order = {
        ui->groupControlBtn, ui->fullScreenBtn,
        m_keymapperBtn, m_gamepadBtn, m_keyboardBtn, m_screenFitBtn,
        ui->volumeUpBtn, ui->volumeDownBtn, m_muteBtn,
        ui->appSwitchBtn, ui->menuBtn, ui->homeBtn, ui->returnBtn
    };
    for (auto *button : order) {
        layout->removeWidget(button);
    }
    // Retirar somente o espaçador visual antigo: os botões novos utilizam
    // o espaço liberado sem modificar a janela de espelhamento.
    for (int i = layout->count() - 1; i >= 0; --i) {
        if (QLayoutItem *item = layout->itemAt(i)) {
            if (item->spacerItem()) delete layout->takeAt(i);
        }
    }
    for (auto *button : order) {
        layout->addWidget(button, 0, Qt::AlignHCenter);
    }

    // Controles descartados da barra do modo tela, sem excluir o
    // código original de comunicação com o Android.
    ui->expandNotifyBtn->hide();
    ui->expandSettingsBtn->hide();
    ui->rotateBtn->hide();
    ui->touchBtn->hide();
    ui->openScreenBtn->hide();
    ui->closeScreenBtn->hide();
    ui->powerBtn->hide();
    ui->screenShotBtn->hide();
    ui->clipboardBtn->hide();

    setFixedWidth(44);
    setObjectName(QStringLiteral("mirroringSideToolbar"));
    setStyleSheet(QStringLiteral(
        "QWidget#mirroringSideToolbar { background-color: #161616; }"
        "QPushButton { border: 0; border-radius: 3px;"
        " background: transparent; color: #ffffff; padding: 0; }"
        "QPushButton:hover:enabled { background-color: #343434; }"
        "QPushButton:pressed { background-color: #494949; }"
        "QPushButton:disabled { color: #b5b5b5; }"
        "QPushButton:checked { background-color: #12622b; color: white; }"
        "QPushButton#groupControlBtn { color: #008c23; }"
    ));
}

bool ToolForm::sendHid(const QByteArray &packet)
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    return device && !device->isCameraMode() && device->sendNativeHidPacket(packet);
}

void ToolForm::toggleGamepad()
{
#ifdef Q_OS_WIN
    if (m_gamepadEnabled) {
        m_gamepadTimer.stop();
        if (m_gamepadRegistered) sendHid(NativeHid::destroy(NativeHid::GamepadId));
        m_gamepadEnabled = false;
        m_gamepadRegistered = false;
        m_lastGamepadReport.clear();
        m_gamepadBtn->setChecked(false);
        m_gamepadBtn->setToolTip(QStringLiteral("Ligar gamepad no Android"));
        return;
    }

    XINPUT_STATE state;
    if (!NativeHid::readGamepad(state)) {
        m_gamepadBtn->setChecked(false);
        QMessageBox::information(this, QStringLiteral("Gamepad"),
            QStringLiteral("Nenhum controle XInput conectado ao computador."));
        return;
    }
    if (!sendHid(NativeHid::createGamepad())) {
        m_gamepadBtn->setChecked(false);
        QMessageBox::warning(this, QStringLiteral("Gamepad"),
            QStringLiteral("Não foi possível abrir o controle virtual no Android."));
        return;
    }
    m_gamepadEnabled = true;
    m_gamepadRegistered = true;
    m_lastGamepadReport.clear();
    m_gamepadBtn->setChecked(true);
    m_gamepadBtn->setToolTip(QStringLiteral("Desligar gamepad no Android"));
    pollGamepad();
    m_gamepadTimer.start();
#else
    m_gamepadBtn->setChecked(false);
    QMessageBox::information(this, QStringLiteral("Gamepad"),
        QStringLiteral("O suporte XInput desta etapa está disponível somente no Windows."));
#endif
}

void ToolForm::pollGamepad()
{
#ifdef Q_OS_WIN
    if (!m_gamepadEnabled) return;
    const auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        m_gamepadTimer.stop();
        m_gamepadEnabled = false;
        m_gamepadRegistered = false;
        m_lastGamepadReport.clear();
        m_gamepadBtn->setChecked(false);
        return;
    }

    XINPUT_STATE state;
    if (!NativeHid::readGamepad(state)) {
        if (m_gamepadRegistered) sendHid(NativeHid::destroy(NativeHid::GamepadId));
        m_gamepadRegistered = false;
        m_lastGamepadReport.clear();
        return;
    }

    if (!m_gamepadRegistered) {
        if (!sendHid(NativeHid::createGamepad())) return;
        m_gamepadRegistered = true;
    }

    const QByteArray report = NativeHid::gamepadReport(state.Gamepad);
    if (report != m_lastGamepadReport) {
        if (sendHid(NativeHid::input(NativeHid::GamepadId, report)))
            m_lastGamepadReport = report;
    }
#endif
}

void ToolForm::togglePhysicalKeyboard()
{
    if (m_keyboardEnabled) {
        if (!sendHid(NativeHid::destroy(NativeHid::KeyboardId))) {
            m_keyboardBtn->setChecked(true);
            return;
        }
        m_keyboardEnabled = false;
        m_keyboardBtn->setChecked(false);
        m_keyboardBtn->setToolTip(QStringLiteral("Usar teclado físico do PC no Android"));
        return;
    }

    // A presença do teclado físico via UHID informa ao Android que existe
    // um teclado de hardware. A digitação continua usando o QtScrcpy original.
    if (!sendHid(NativeHid::createKeyboard())) {
        m_keyboardBtn->setChecked(false);
        QMessageBox::warning(this, QStringLiteral("Teclado físico"),
            QStringLiteral("Não foi possível registrar o teclado físico no Android."));
        return;
    }
    m_keyboardEnabled = true;
    m_keyboardBtn->setChecked(true);
    m_keyboardBtn->setToolTip(QStringLiteral("Reativar o teclado virtual do Android"));
}

void ToolForm::stopNativeInput()
{
    m_gamepadTimer.stop();
    if (m_gamepadRegistered) sendHid(NativeHid::destroy(NativeHid::GamepadId));
    if (m_keyboardEnabled) sendHid(NativeHid::destroy(NativeHid::KeyboardId));
    m_gamepadEnabled = false;
    m_gamepadRegistered = false;
    m_keyboardEnabled = false;
    m_lastGamepadReport.clear();
    if (m_gamepadBtn) m_gamepadBtn->setChecked(false);
    if (m_keyboardBtn) m_keyboardBtn->setChecked(false);
}

void ToolForm::updateCameraMode()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    const bool camera = device && device->isCameraMode();

    ui->groupControlBtn->setVisible(!camera);
    ui->expandNotifyBtn->setVisible(!camera);
    ui->expandSettingsBtn->setVisible(!camera);
    ui->rotateBtn->setVisible(!camera);
    ui->touchBtn->setVisible(!camera);
    ui->openScreenBtn->setVisible(!camera);
    ui->closeScreenBtn->setVisible(!camera);
    ui->powerBtn->setVisible(!camera);
    ui->volumeUpBtn->setVisible(!camera);
    ui->volumeDownBtn->setVisible(!camera);
    ui->appSwitchBtn->setVisible(!camera);
    ui->menuBtn->setVisible(!camera);
    ui->homeBtn->setVisible(!camera);
    ui->returnBtn->setVisible(!camera);
    ui->clipboardBtn->setVisible(!camera);
    ui->cameraTorchBtn->setVisible(camera);
    ui->cameraZoomOutBtn->setVisible(camera);
    ui->cameraZoomInBtn->setVisible(camera);

    ui->expandNotifyBtn->hide();
    ui->expandSettingsBtn->hide();
    ui->rotateBtn->hide();
    ui->touchBtn->hide();
    ui->openScreenBtn->hide();
    ui->closeScreenBtn->hide();
    ui->powerBtn->hide();
    if (m_keymapperBtn) m_keymapperBtn->setVisible(!camera);
    if (m_gamepadBtn) m_gamepadBtn->setVisible(!camera);
    if (m_keyboardBtn) m_keyboardBtn->setVisible(!camera);
    if (m_screenFitBtn) m_screenFitBtn->setVisible(!camera);
    if (m_muteBtn) m_muteBtn->setVisible(!camera);
}

void ToolForm::initStyle()
{
    IconHelper::Instance()->SetIcon(ui->fullScreenBtn, QChar(0xf0b2), 15);
    IconHelper::Instance()->SetIcon(ui->menuBtn, QChar(0xf096), 15);
    IconHelper::Instance()->SetIcon(ui->homeBtn, QChar(0xf1db), 15);
    //IconHelper::Instance()->SetIcon(ui->returnBtn, QChar(0xf104), 15);
    IconHelper::Instance()->SetIcon(ui->returnBtn, QChar(0xf053), 15);
    IconHelper::Instance()->SetIcon(ui->appSwitchBtn, QChar(0xf24d), 15);
    IconHelper::Instance()->SetIcon(ui->volumeUpBtn, QChar(0xf028), 15);
    IconHelper::Instance()->SetIcon(ui->volumeDownBtn, QChar(0xf027), 15);
    IconHelper::Instance()->SetIcon(ui->openScreenBtn, QChar(0xf06e), 15);
    IconHelper::Instance()->SetIcon(ui->closeScreenBtn, QChar(0xf070), 15);
    IconHelper::Instance()->SetIcon(ui->powerBtn, QChar(0xf011), 15);
    IconHelper::Instance()->SetIcon(ui->expandNotifyBtn, QChar(0xf103), 15);
    IconHelper::Instance()->SetIcon(ui->expandSettingsBtn, QChar(0xf013), 15);
    IconHelper::Instance()->SetIcon(ui->rotateBtn, QChar(0xf021), 15);
    IconHelper::Instance()->SetIcon(ui->screenShotBtn, QChar(0xf0c4), 15);
    IconHelper::Instance()->SetIcon(ui->touchBtn, QChar(0xf111), 15);
    IconHelper::Instance()->SetIcon(ui->groupControlBtn, QChar(0xf0c0), 15);
    IconHelper::Instance()->SetIcon(ui->clipboardBtn, QChar(0xf0c5), 15);
    IconHelper::Instance()->SetIcon(ui->cameraTorchBtn, QChar(0xf0eb), 15);
    IconHelper::Instance()->SetIcon(ui->cameraZoomOutBtn, QChar(0xf010), 15);
    IconHelper::Instance()->SetIcon(ui->cameraZoomInBtn, QChar(0xf00e), 15);
    // Ícones da barra redesenhada, usando a fonte já incluída no QtScrcpy.
    IconHelper::Instance()->SetIcon(m_keymapperBtn, QChar(0xf11c), 17);
    IconHelper::Instance()->SetIcon(m_gamepadBtn, QChar(0xf11b), 17);
    IconHelper::Instance()->SetIcon(m_keyboardBtn, QChar(0xf10b), 20);
    IconHelper::Instance()->SetIcon(m_screenFitBtn, QChar(0xf065), 18);
    IconHelper::Instance()->SetIcon(m_muteBtn, QChar(0xf026), 18);
}

void ToolForm::updateGroupControl()
{
    if (m_isHost) {
        ui->groupControlBtn->setStyleSheet("color: red");
    } else {
        ui->groupControlBtn->setStyleSheet("color: green");
    }

    GroupController::instance().updateDeviceState(m_serial);
}

void ToolForm::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
#else
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
#endif
        event->accept();
    }
}

void ToolForm::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void ToolForm::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        move(event->globalPos() - m_dragPosition);
#else
        move(event->globalPosition().toPoint() - m_dragPosition);
#endif
        event->accept();
    }
}

void ToolForm::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    qDebug() << "show event";
}

void ToolForm::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    qDebug() << "hide event";
}

void ToolForm::on_fullScreenBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }

    dynamic_cast<VideoForm*>(parent())->switchFullScreen();
}

void ToolForm::on_returnBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->postGoBack();
}

void ToolForm::on_homeBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->postGoHome();
}

void ToolForm::on_menuBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->postGoMenu();
}

void ToolForm::on_appSwitchBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->postAppSwitch();
}

void ToolForm::on_powerBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->postPower();
}

void ToolForm::on_screenShotBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->screenshot();
}

void ToolForm::on_volumeUpBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->postVolumeUp();
}

void ToolForm::on_volumeDownBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->postVolumeDown();
}

void ToolForm::on_closeScreenBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->setDisplayPower(false);
}

void ToolForm::on_expandNotifyBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->expandNotificationPanel();
}

void ToolForm::on_expandSettingsBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (device) {
        device->expandSettingsPanel();
    }
}

void ToolForm::on_rotateBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (device) {
        device->rotateDevice();
    }
}

void ToolForm::on_touchBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }

    m_showTouch = !m_showTouch;
    device->showTouch(m_showTouch);
}

void ToolForm::on_cameraTorchBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device || !device->isCameraMode()) {
        return;
    }
    m_cameraTorch = !m_cameraTorch;
    device->setCameraTorch(m_cameraTorch);
    ui->cameraTorchBtn->setStyleSheet(m_cameraTorch ? "color: #f0c419" : "");
}

void ToolForm::on_cameraZoomOutBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (device && device->isCameraMode()) {
        device->cameraZoomOut();
    }
}

void ToolForm::on_cameraZoomInBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (device && device->isCameraMode()) {
        device->cameraZoomIn();
    }
}

void ToolForm::on_groupControlBtn_clicked()
{
    m_isHost = !m_isHost;
    updateGroupControl();
}

void ToolForm::on_openScreenBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->setDisplayPower(true);
}

void ToolForm::on_clipboardBtn_clicked()
{
    auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) {
        return;
    }
    device->requestDeviceClipboard();
}