#include "mainwindow.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

namespace {
QPushButton *makeButton(const QString &label, QWidget *parent) {
    auto *button = new QPushButton(label, parent);
    button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    return button;
}
void appendButton(QHBoxLayout *layout, QPushButton *button) {
    layout->addWidget(button);
}
QStringList parseArguments(const QString &command) {
    return QProcess::splitCommand(command.trimmed());
}
}

MainWindow::MainWindow() : settings("WELLS", "WELLS CONECT") {
    setWindowTitle("WELLS Conect");
    resize(650, 580);
    setMinimumSize(600, 520);
    auto *root = new QWidget(this);
    setCentralWidget(root);
    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(17, 13, 17, 13);
    layout->setSpacing(10);

    auto *heading = new QHBoxLayout;
    auto *brand = new QLabel("WELLS CONECT", root);
    brand->setObjectName("brand");
    auto *gear = makeButton("⚙", root);
    gear->setObjectName("settingsButton");
    gear->setToolTip("Definições Globais");
    heading->addWidget(brand);
    heading->addStretch();
    heading->addWidget(gear);
    layout->addLayout(heading);
    connect(gear, &QPushButton::clicked, this, &MainWindow::openSettings);

    auto *connection = new QGroupBox(root);
    auto *connectionLayout = new QVBoxLayout(connection);
    auto *top = new QHBoxLayout;
    auto *wifi = makeButton("Conectar via Wi-Fi", connection);
    auto *usb = makeButton("Conectar via USB", connection);
    appendButton(top, wifi);
    appendButton(top, usb);
    top->addStretch();
    connectionLayout->addLayout(top);

    auto *hint = new QLabel("Clique duas vezes para conectar:", connection);
    connectionLayout->addWidget(hint);
    devices = new QListWidget(connection);
    devices->setMinimumHeight(118);
    connectionLayout->addWidget(devices);
    layout->addWidget(connection);

    auto *usbGroup = new QGroupBox("Conexão USB", root);
    auto *usbLayout = new QVBoxLayout(usbGroup);
    auto *row1 = new QHBoxLayout;
    deviceName = new QLineEdit(usbGroup);
    deviceName->setReadOnly(true);
    auto *nameBtn = makeButton("Atualizar nome", usbGroup);
    row1->addWidget(new QLabel("Nome do dispositivo:", usbGroup));
    row1->addWidget(deviceName, 1);
    row1->addWidget(nameBtn);
    usbLayout->addLayout(row1);

    auto *row2 = new QHBoxLayout;
    serials = new QComboBox(usbGroup);
    serials->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    row2->addWidget(new QLabel("Identificador:", usbGroup));
    row2->addWidget(serials, 1);
    usbLayout->addLayout(row2);

    auto *serverRow = new QHBoxLayout;
    auto *start = makeButton("Iniciar servidor", usbGroup);
    auto *stop = makeButton("Parar servidor", usbGroup);
    serverRow->addWidget(start);
    serverRow->addWidget(stop);
    serverRow->addStretch();
    usbLayout->addLayout(serverRow);

    auto *deviceRow = new QHBoxLayout;
    auto *stopAll = makeButton("Parar todos os servidores", usbGroup);
    auto *refresh = makeButton("Atualizar dispositivos", usbGroup);
    auto *ipBtn = makeButton("Consultar IP", usbGroup);
    auto *tcpBtn = makeButton("Iniciar ADB no dispositivo", usbGroup);
    for (auto *b : {stopAll, refresh, ipBtn, tcpBtn}) deviceRow->addWidget(b);
    usbLayout->addLayout(deviceRow);

    auto *misc = new QHBoxLayout;
    auto *install = makeButton("Instalar sndcpy", usbGroup);
    auto *audioStart = makeButton("Iniciar áudio", usbGroup);
    auto *audioStop = makeButton("Parar áudio", usbGroup);
    misc->addWidget(install);
    misc->addWidget(audioStart);
    misc->addWidget(audioStop);
    misc->addStretch();
    usbLayout->addLayout(misc);
    // Scrcpy 5.0.1 encaminha áudio por seu próprio processo no Android 11+.
    // Esses botões herdados do modelo permanecem identificados, mas sem
    // simular um serviço sndcpy inexistente nesta primeira base.
    for (auto *b : {install, audioStart, audioStop}) b->setEnabled(false);
    install->setToolTip("Compatibilidade sndcpy: pendente");
    audioStart->setToolTip("O áudio é iniciado automaticamente com scrcpy 5.0.1 quando suportado");
    audioStop->setToolTip("O áudio acompanha a sessão scrcpy; desligue a sessão para encerrá-lo");
    layout->addWidget(usbGroup);

    auto *wireless = new QGroupBox("Sem fio", root);
    auto *wirelessRow = new QHBoxLayout(wireless);
    ip = new QLineEdit(wireless);
    ip->setPlaceholderText("192.168.1.123");
    ip->setText(settings.value("wifi/ip").toString());
    port = new QLineEdit(wireless);
    port->setText(settings.value("wifi/port", "5555").toString());
    port->setMaximumWidth(72);
    auto *connectWifi = makeButton("Conectar sem fio", wireless);
    auto *disconnectWifi = makeButton("Desconectar rede sem fio", wireless);
    wirelessRow->addWidget(ip, 2);
    wirelessRow->addWidget(port, 0);
    wirelessRow->addWidget(connectWifi);
    wirelessRow->addWidget(disconnectWifi);
    layout->addWidget(wireless);
    layout->addStretch();

    auto startSelected = [this]() {
        if (selectedSerial().isEmpty()) {
            QMessageBox::information(this, "WELLS CONECT", "Selecione um dispositivo conectado.");
            return;
        }
        launchScrcpy(selectedSerial());
    };
    connect(usb, &QPushButton::clicked, this, startSelected);
    connect(wifi, &QPushButton::clicked, this, [this, startSelected]() {
        const auto serial = selectedSerial();
        if (serial.contains(':')) startSelected();
        else if (!ip->text().trimmed().isEmpty()) {
            runAdb({"connect", ip->text().trimmed() + ":" + port->text().trimmed()}, true);
        } else {
            QMessageBox::information(this, "Conexão Wi-Fi", "Informe o IP na seção Sem fio.");
        }
    });
    connect(devices, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (item) launchScrcpy(item->data(Qt::UserRole).toString());
    });
    connect(devices, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem *item, QListWidgetItem *) {
        if (!item) return;
        const int index = serials->findText(item->data(Qt::UserRole).toString());
        if (index >= 0) serials->setCurrentIndex(index);
    });
    connect(serials, &QComboBox::currentTextChanged, this, [this](const QString &serial) {
        if (!serial.isEmpty()) runAdb({"-s", serial, "shell", "getprop", "ro.product.model"});
    });
    connect(refresh, &QPushButton::clicked, this, &MainWindow::refreshDevices);
    connect(nameBtn, &QPushButton::clicked, this, [this]() {
        if (!selectedSerial().isEmpty())
            runAdb({"-s", selectedSerial(), "shell", "getprop", "ro.product.model"});
    });
    connect(start, &QPushButton::clicked, this, startSelected);
    connect(stop, &QPushButton::clicked, this, [this]() {
        const QString serial = selectedSerial();
        for (auto *p : sessions) {
            if (p && p->property("serial").toString() == serial && p->state() != QProcess::NotRunning)
                p->terminate();
        }
    });
    connect(stopAll, &QPushButton::clicked, this, [this]() {
        for (auto *p : sessions) if (p && p->state() != QProcess::NotRunning) p->terminate();
    });
    connect(ipBtn, &QPushButton::clicked, this, [this]() {
        if (!selectedSerial().isEmpty()) runAdb({"-s", selectedSerial(), "shell", "ip", "route"});
    });
    connect(tcpBtn, &QPushButton::clicked, this, [this]() {
        if (!selectedSerial().isEmpty()) runAdb({"-s", selectedSerial(), "tcpip", port->text().trimmed()});
    });
    connect(connectWifi, &QPushButton::clicked, this, [this]() {
        const QString host = ip->text().trimmed();
        const QString value = port->text().trimmed();
        QRegularExpression ipv4(R"(^(?:\d{1,3}\.){3}\d{1,3}$)");
        if (!ipv4.match(host).hasMatch()) { log("Informe um IPv4 válido."); return; }
        settings.setValue("wifi/ip", host);
        settings.setValue("wifi/port", value);
        runAdb({"connect", host + ":" + value}, true);
    });
    connect(disconnectWifi, &QPushButton::clicked, this, [this]() {
        if (ip->text().trimmed().isEmpty()) return;
        runAdb({"disconnect", ip->text().trimmed() + ":" + port->text().trimmed()}, true);
    });
    refreshDevices();
}

QString MainWindow::scrcpyPath() const {
#ifdef Q_OS_WIN
    return QDir(QCoreApplication::applicationDirPath()).filePath("scrcpy/scrcpy.exe");
#else
    return QDir(QCoreApplication::applicationDirPath()).filePath("scrcpy/scrcpy");
#endif
}
QString MainWindow::adbPath() const {
#ifdef Q_OS_WIN
    return QDir(QCoreApplication::applicationDirPath()).filePath("scrcpy/adb.exe");
#else
    return "adb";
#endif
}
QString MainWindow::selectedSerial() const { return serials ? serials->currentText() : QString(); }
void MainWindow::log(const QString &message) {
    if (output) output->append(message.toHtmlEscaped());
    else qInfo("%s", qPrintable(message));
}
void MainWindow::runAdb(const QStringList &args, bool refresh) {
    if (!QFileInfo::exists(adbPath())) { log("ADB não encontrado: " + adbPath()); return; }
    auto *process = new QProcess(this);
    process->setProgram(adbPath());
    process->setArguments(args);
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, &QProcess::finished, this, [this, process, args, refresh](int code) {
        const QString result = QString::fromLocal8Bit(process->readAll()).trimmed();
        if (args == QStringList({"devices", "-l"})) populateDevices(result);
        else if (args.contains("ro.product.model")) deviceName->setText(result);
        log("adb " + args.join(' ') + " [código " + QString::number(code) + "]\n" + result);
        process->deleteLater();
        if (refresh) refreshDevices();
    });
    process->start();
}
void MainWindow::populateDevices(const QString &text) {
    devices->clear();
    serials->clear();
    for (const QString &line : text.split('\n')) {
        const QString trimmed = line.trimmed();
        const QStringList parts = trimmed.split(QRegularExpression(R"(\s+)"), Qt::SkipEmptyParts);
        if (parts.size() < 2 || parts.at(1) != "device") continue;
        const QString serial = parts.at(0);
        QString label = serial;
        for (const QString &part : parts) if (part.startsWith("model:")) label = part.mid(6) + " - " + serial;
        auto *item = new QListWidgetItem(label, devices);
        item->setData(Qt::UserRole, serial);
        serials->addItem(serial);
    }
}
void MainWindow::refreshDevices() { runAdb({"devices", "-l"}); }
void MainWindow::launchScrcpy(const QString &serial) {
    if (serial.isEmpty()) return;
    if (!QFileInfo::exists(scrcpyPath())) {
        QMessageBox::warning(this, "WELLS CONECT",
            "scrcpy 5.0.1 não encontrado no pacote: " + scrcpyPath());
        return;
    }
    QStringList args = {"--serial", serial, "--window-title", "WELLS Conect - " + serial};
    const int size = settings.value("video/maxSize", 720).toInt();
    const int fps = settings.value("video/maxFps", 60).toInt();
    const int bits = settings.value("video/bitrate", 8).toInt();
    if (size > 0) args << "--max-size" << QString::number(size);
    if (fps > 0) args << "--max-fps" << QString::number(fps);
    args << "--video-bit-rate" << QString::number(bits) + "M";
    if (settings.value("window/stayOnTop", false).toBool()) args << "--always-on-top";
    if (settings.value("video/showFps", false).toBool()) args << "--print-fps";
    if (settings.value("video/screenOff", false).toBool()) args << "--turn-screen-off";
    auto *process = new QProcess(this);
    process->setProperty("serial", serial);
    process->setProgram(scrcpyPath());
    process->setArguments(args);
    process->setWorkingDirectory(QFileInfo(scrcpyPath()).absolutePath());
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, &QProcess::readyRead, this, [this, process]() {
        log(QString::fromLocal8Bit(process->readAll()).trimmed());
    });
    connect(process, &QProcess::finished, this, [this, process](int code) {
        log("Sessão encerrada (" + QString::number(code) + ")");
        sessions.removeAll(process);
        process->deleteLater();
    });
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError) {
        log("Erro ao executar scrcpy: " + process->errorString());
    });
    sessions.append(process);
    process->start();
    log("Iniciando scrcpy 5.0.1 para " + serial);
}
void MainWindow::openSettings() {
    if (settingsWindow) { settingsWindow->show(); settingsWindow->raise(); settingsWindow->activateWindow(); return; }
    auto *dialog = new QDialog(this, Qt::Window);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle("WELLS Conect — Definições Globais");
    dialog->resize(650, 560);
    settingsWindow = dialog;
    auto *root = new QVBoxLayout(dialog);
    auto *brand = new QLabel("WELLS CONECT", dialog);
    brand->setObjectName("settingsBrand");
    brand->setAlignment(Qt::AlignCenter);
    root->addWidget(brand);
    auto *tabs = new QTabWidget(dialog);
    root->addWidget(tabs, 1);
    auto *initial = new QWidget(tabs);
    auto *form = new QFormLayout(initial);
    auto *source = new QComboBox(initial);
    source->addItems({"Tela"});
    form->addRow("Fonte de vídeo:", source);
    bitrate = new QSpinBox(initial);
    bitrate->setRange(1, 100);
    bitrate->setSuffix(" Mbps");
    bitrate->setValue(settings.value("video/bitrate", 8).toInt());
    form->addRow("Taxa de bits:", bitrate);
    maxSize = new QSpinBox(initial);
    maxSize->setRange(0, 3840);
    maxSize->setSingleStep(120);
    maxSize->setSpecialValueText("Automático");
    maxSize->setValue(settings.value("video/maxSize", 720).toInt());
    form->addRow("Resolução máxima:", maxSize);
    maxFps = new QSpinBox(initial);
    maxFps->setRange(0, 120);
    maxFps->setSpecialValueText("Automático");
    maxFps->setValue(settings.value("video/maxFps", 60).toInt());
    form->addRow("FPS máximo:", maxFps);
    auto *codec = new QComboBox(initial);
    codec->addItems({"Padrão (automático)"});
    form->addRow("Modo de codec:", codec);
    auto *record = new QComboBox(initial);
    record->addItems({"mp4"});
    form->addRow("Formato de gravação:", record);
    auto *orientation = new QComboBox(initial);
    orientation->addItems({"Sem bloqueio"});
    form->addRow("Bloquear orientação:", orientation);
    showFps = new QCheckBox("Mostrar FPS", initial);
    showFps->setChecked(settings.value("video/showFps", false).toBool());
    form->addRow(showFps);
    stayOnTop = new QCheckBox("Sempre no topo", initial);
    stayOnTop->setChecked(settings.value("window/stayOnTop", false).toBool());
    form->addRow(stayOnTop);
    turnScreenOff = new QCheckBox("Tela apagada no Android", initial);
    turnScreenOff->setChecked(settings.value("video/screenOff", false).toBool());
    form->addRow(turnScreenOff);
    form->addRow(new QLabel("Outras opções do QtScrcpy serão integradas por etapas.", initial));
    tabs->addTab(initial, "Configurações iniciais");
    auto *display = new QWidget(tabs);
    auto *displayLayout = new QVBoxLayout(display);
    displayLayout->addWidget(new QLabel("Exibição avançada: recursos pendentes de integração.", display));
    displayLayout->addStretch();
    tabs->addTab(display, "Exibição avançada");
    auto *advanced = new QWidget(tabs);
    auto *advLayout = new QVBoxLayout(advanced);
    advLayout->addWidget(new QLabel("Configurações avançadas: recursos pendentes de integração.", advanced));
    advLayout->addStretch();
    tabs->addTab(advanced, "Configurações avançadas");

    auto *adbBox = new QGroupBox("adb", dialog);
    auto *adbLayout = new QVBoxLayout(adbBox);
    auto *adbRow = new QHBoxLayout;
    adbCommand = new QLineEdit("devices", adbBox);
    auto *run = makeButton("Executar", adbBox);
    auto *kill = makeButton("Encerrar", adbBox);
    auto *clear = makeButton("Limpar", adbBox);
    adbRow->addWidget(new QLabel("Comando ADB:", adbBox));
    adbRow->addWidget(adbCommand, 1);
    adbRow->addWidget(run);
    adbRow->addWidget(kill);
    adbRow->addWidget(clear);
    adbLayout->addLayout(adbRow);
    output = new QTextEdit(adbBox);
    output->setReadOnly(true);
    output->setMinimumHeight(100);
    adbLayout->addWidget(output);
    root->addWidget(adbBox);
    connect(run, &QPushButton::clicked, this, [this]() {
        if (!adbCommand->text().trimmed().isEmpty()) runAdb(parseArguments(adbCommand->text()));
    });
    connect(kill, &QPushButton::clicked, this, [this]() { runAdb({"kill-server"}); });
    connect(clear, &QPushButton::clicked, output, &QTextEdit::clear);
    auto *applyRow = new QHBoxLayout;
    auto *apply = makeButton("Aplicar", dialog);
    applyRow->addStretch();
    applyRow->addWidget(apply);
    root->addLayout(applyRow);
    connect(apply, &QPushButton::clicked, this, [this]() {
        settings.setValue("video/bitrate", bitrate->value());
        settings.setValue("video/maxSize", maxSize->value());
        settings.setValue("video/maxFps", maxFps->value());
        settings.setValue("video/showFps", showFps->isChecked());
        settings.setValue("window/stayOnTop", stayOnTop->isChecked());
        settings.setValue("video/screenOff", turnScreenOff->isChecked());
        log("Configurações aplicadas às próximas conexões.");
    });
    connect(dialog, &QObject::destroyed, this, [this]() {
        output = nullptr;
        adbCommand = nullptr;
        bitrate = maxSize = maxFps = nullptr;
        stayOnTop = turnScreenOff = showFps = nullptr;
    });
    dialog->show();
}
