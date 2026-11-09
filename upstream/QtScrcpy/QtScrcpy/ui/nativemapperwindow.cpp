#include "nativemapperwindow.h"
#include "../QtScrcpyCore/include/QtScrcpyCore.h"

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QBuffer>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlRequestInfo>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlScheme>
#include <QWebEngineView>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QUrl>

namespace {
const char *kMapperScheme = "qtscrcpymapper";
class MapperOfflineInterceptor final : public QWebEngineUrlRequestInterceptor {
public:
    using QWebEngineUrlRequestInterceptor::QWebEngineUrlRequestInterceptor;
    void interceptRequest(QWebEngineUrlRequestInfo &info) override {
        const QString scheme = info.requestUrl().scheme().toLower();
        if (scheme == QStringLiteral("http") || scheme == QStringLiteral("https")) {
            info.block(true);
        }
    }
};


QString currentMapDirectory() {
    QString directory = QString::fromLocal8Bit(qgetenv("QTSCRCPY_KEYMAP_PATH"));
    if (directory.isEmpty()) {
        directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                  + QStringLiteral("/keymap");
    }
    return directory;
}

QString mapperDirectory() {
    const QString bundled = QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("ScrcpyKeyMapper"));
    if (QFileInfo::exists(QDir(bundled).filePath(QStringLiteral("index.html")))) {
        return bundled;
    }
    // Apenas para uso na árvore original durante desenvolvimento.
    const QString source = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(
        QStringLiteral("../../../../ScrcpyKeyMapper"));
    return source;
}

} // namespace

void registerEmbeddedMapperScheme() {
    QWebEngineUrlScheme scheme(kMapperScheme);
    scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort);
    scheme.setFlags(QWebEngineUrlScheme::SecureScheme
                  | QWebEngineUrlScheme::LocalScheme
                  | QWebEngineUrlScheme::LocalAccessAllowed
                  | QWebEngineUrlScheme::CorsEnabled
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
                  | QWebEngineUrlScheme::FetchApiAllowed
#endif
                  );
    QWebEngineUrlScheme::registerScheme(scheme);
}

MapperAssetHandler::MapperAssetHandler(const QString &root, QObject *parent)
    : QWebEngineUrlSchemeHandler(parent), m_root(QDir(root).canonicalPath()) {}

void MapperAssetHandler::requestStarted(QWebEngineUrlRequestJob *request) {
    const QString relative = request->requestUrl().path().mid(1);
    const QString filePath = QFileInfo(QDir(m_root).filePath(relative)).canonicalFilePath();
    if (m_root.isEmpty() || filePath.isEmpty()
        || !filePath.startsWith(m_root + QDir::separator())) {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }
    QFile *file = new QFile(filePath, request);
    if (!file->open(QIODevice::ReadOnly)) {
        request->fail(QWebEngineUrlRequestJob::RequestFailed);
        return;
    }
    // ES modules do Chromium exigem MIME JavaScript explícito. No Qt 5
    // o reconhecimento do tipo de .js pode variar conforme o Windows.
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    QByteArray mime;
    if (suffix == QStringLiteral("js") || suffix == QStringLiteral("mjs")) {
        mime = QByteArrayLiteral("text/javascript");
    } else if (suffix == QStringLiteral("css")) {
        mime = QByteArrayLiteral("text/css");
    } else if (suffix == QStringLiteral("html")) {
        mime = QByteArrayLiteral("text/html");
    } else if (suffix == QStringLiteral("json")) {
        mime = QByteArrayLiteral("application/json");
    } else {
        mime = QMimeDatabase().mimeTypeForFile(filePath).name().toUtf8();
    }
    request->reply(mime, file);
}

NativeMapperWindow::NativeMapperWindow(const QString &deviceSerial)
    : QDialog(nullptr), m_serial(deviceSerial) {
    setWindowTitle(QStringLiteral("ScrcpyKeyMapper — ") + m_serial);
    // Editor independente; nao assumir tamanho superior ao monitor de 1280x720.
    const QRect usable = QGuiApplication::primaryScreen()
        ? QGuiApplication::primaryScreen()->availableGeometry()
        : QRect(0, 0, 920, 620);
    const QSize initial = QSize(920, 620).boundedTo(usable.size());
    setMinimumSize(600, 400);
    resize(initial);
    move(usable.center() - rect().center());
    setAttribute(Qt::WA_DeleteOnClose);

    auto *outer = new QVBoxLayout(this);
    auto *toolbar = new QHBoxLayout();
    m_profiles = new QComboBox(this);
    m_profiles->setMinimumWidth(240);
    toolbar->addWidget(new QLabel(QStringLiteral("Perfil do jogo:"), this));
    toolbar->addWidget(m_profiles);
    auto *newButton = new QPushButton(QStringLiteral("Novo perfil"), this);
    auto *refreshButton = new QPushButton(QStringLiteral("Atualizar perfis"), this);
    toolbar->addWidget(newButton);
    toolbar->addWidget(refreshButton);
    toolbar->addStretch();
    m_status = new QLabel(QStringLiteral("Editor original ScrcpyKeyMapper"), this);
    toolbar->addWidget(m_status);
    outer->addLayout(toolbar);

    m_view = new QWebEngineView(this);
    outer->addWidget(m_view, 1);
    // Não permitir requisições da página a websites.
    m_view->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    m_view->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    m_view->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    // Perfil WebEngine exclusivo da janela: não usa o navegador do sistema.
    auto *profile = new QWebEngineProfile(this);
    profile->setHttpCacheType(QWebEngineProfile::NoCache);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    // A página do editor é servida somente no loopback local. Nenhuma
    // requisição para a internet é autorizada pelo perfil do WebEngine.
    auto *assetServer = new QTcpServer(this);
    const QString assetRoot = QDir(mapperDirectory()).canonicalPath();
    if (assetRoot.isEmpty() || !assetServer->listen(QHostAddress::LocalHost, 0)) {
        m_status->setText(QStringLiteral("Erro ao iniciar recursos locais do mapeador"));
        return;
    }
    const quint16 assetPort = assetServer->serverPort();
    QObject::connect(assetServer, &QTcpServer::newConnection, this,
            [this, assetServer, assetRoot]() {
        while (assetServer->hasPendingConnections()) {
            auto *socket = assetServer->nextPendingConnection();
            socket->setParent(assetServer);
            QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
            QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, assetRoot]() {
                if (socket->property("mapperReplied").toBool()) return;
                QByteArray received = socket->property("mapperRequest").toByteArray();
                received += socket->readAll();
                if (received.size() > 16384) {
                    socket->disconnectFromHost();
                    return;
                }
                socket->setProperty("mapperRequest", received);
                if (!received.contains("\r\n\r\n")) return;
                socket->setProperty("mapperReplied", true);
                const QByteArray requestLine = received.left(received.indexOf("\r\n"));
                const QList<QByteArray> fields = requestLine.split(' ');
                QByteArray content;
                QByteArray mime = QByteArrayLiteral("application/octet-stream");
                QByteArray status = QByteArrayLiteral("200 OK");
                if (fields.size() < 3 || fields.at(0) != "GET") {
                    status = QByteArrayLiteral("405 Method Not Allowed");
                } else {
                    const QByteArray rawPath = fields.at(1).split('?').first();
                    const QString path = QUrl::fromPercentEncoding(rawPath);
                    const QString relative = path.startsWith('/') ? path.mid(1) : path;
                    const QString selected = relative.isEmpty()
                        ? QStringLiteral("index.html") : relative;
                    QString filePath;
                    if (selected == QStringLiteral("qtwebchannel/qwebchannel.js")) {
                        filePath = QStringLiteral(":/qtwebchannel/qwebchannel.js");
                    } else {
                        filePath = QFileInfo(QDir(assetRoot).filePath(selected)).canonicalFilePath();
                        if (!filePath.startsWith(assetRoot + QDir::separator()))
                            filePath.clear();
                    }
                    QFile file(filePath);
                    if (filePath.isEmpty() || !file.open(QIODevice::ReadOnly)) {
                        status = QByteArrayLiteral("404 Not Found");
                    } else {
                        content = file.readAll();
                        const QString suffix = QFileInfo(filePath).suffix().toLower();
                        if (suffix == QStringLiteral("js") || suffix == QStringLiteral("mjs"))
                            mime = QByteArrayLiteral("text/javascript");
                        else if (suffix == QStringLiteral("html"))
                            mime = QByteArrayLiteral("text/html; charset=utf-8");
                        else if (suffix == QStringLiteral("css"))
                            mime = QByteArrayLiteral("text/css");
                        else if (suffix == QStringLiteral("json"))
                            mime = QByteArrayLiteral("application/json");
                        else if (suffix == QStringLiteral("svg"))
                            mime = QByteArrayLiteral("image/svg+xml");
                        else if (suffix == QStringLiteral("png"))
                            mime = QByteArrayLiteral("image/png");
                    }
                }
                const QByteArray header = QByteArrayLiteral("HTTP/1.1 ") + status
                    + QByteArrayLiteral("\r\nContent-Type: ") + mime
                    + QByteArrayLiteral("\r\nContent-Length: ")
                    + QByteArray::number(content.size())
                    + QByteArrayLiteral("\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nConnection: close\r\n\r\n");
                socket->write(header);
                socket->write(content);
                socket->disconnectFromHost();
            });
        }
    });
    class LoopbackOnly final : public QWebEngineUrlRequestInterceptor {
    public:
        explicit LoopbackOnly(quint16 port, QObject *parent)
            : QWebEngineUrlRequestInterceptor(parent), m_port(port) {}
        void interceptRequest(QWebEngineUrlRequestInfo &info) override {
            const QUrl url = info.requestUrl();
            if (url.scheme() != QStringLiteral("http")
                || url.host() != QStringLiteral("127.0.0.1")
                || url.port() != m_port)
                info.block(true);
        }
    private:
        quint16 m_port;
    };
    profile->setUrlRequestInterceptor(new LoopbackOnly(assetPort, profile));
    auto *page = new QWebEnginePage(profile, m_view);
    m_view->setPage(page);
    auto *channel = new QWebChannel(m_view);
    channel->registerObject(QStringLiteral("nativeMapper"), this);
    page->setWebChannel(channel);

    connect(m_profiles, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { loadSelectedProfile(); });
    connect(newButton, &QPushButton::clicked, this, &NativeMapperWindow::createProfile);
    connect(refreshButton, &QPushButton::clicked, this, &NativeMapperWindow::refreshProfiles);

    connect(m_view, &QWebEngineView::loadFinished, this, [this](bool ok) {
        if (!ok) {
            m_status->setText(QStringLiteral("Erro ao abrir recursos locais do mapeador"));
            return;
        }
        // Nenhuma captura acontece na abertura: só inicializar o editor
        // e aguardar o usuário clicar em Background.
        const QString script = QStringLiteral(R"JS(
            (function waitMapper(attempt) {
                if (!window.nodeManager || !window.mapperApplyScreenshotData) {
                    if (attempt < 160) setTimeout(function(){ waitMapper(attempt + 1); }, 40);
                    else console.error('Canvas do mapeador nao inicializado.');
                    return;
                }
                window.mapperReady = true;
                if (window.mapperInitialProfile) window.mapperLoadProfile(window.mapperInitialProfile);
            })(0);
        )JS");
        m_view->page()->runJavaScript(script);
        loadSelectedProfile();
    });

    refreshProfiles();
    // Sem dependência de navegador externo: todos os arquivos necessários
    // precisam estar presentes dentro do pacote distribuído.
    const QString base = mapperDirectory();
    const QStringList requiredAssets = {
        QStringLiteral("index.html"),
        QStringLiteral("js/app.js"),
        QStringLiteral("js/native-bridge.js"),
        QStringLiteral("vendor/konva.min.js"),
        QStringLiteral("vendor/bootstrap.min.css"),
        QStringLiteral("vendor/bootstrap.bundle.min.js")
    };
    for (const QString &asset : requiredAssets) {
        if (!QFileInfo::exists(QDir(base).filePath(asset))) {
            m_status->setText(QStringLiteral("Arquivo local ausente: ") + asset);
            m_view->setHtml(QStringLiteral(
                "<h3>Editor incompleto</h3><p>Dependência local ausente: %1</p>")
                .arg(asset.toHtmlEscaped()));
            return;
        }
    }
    // Servir HTML, módulos ES e imports pelo MESMO esquema registrado.
    // file:// não forneceu MIME correto aos módulos no Qt WebEngine 5.15.
    m_view->load(QUrl(QStringLiteral("http://127.0.0.1:%1/index.html").arg(assetPort)));
}

QString NativeMapperWindow::screenData() const {
    // Obter uma cópia do último quadro RGB32 pelo decodificador que o QtScrcpy
    // já usa para screenshots. Não acessa OpenGL nem grava imagem em disco.
    const auto device = qsc::IDeviceManage::getInstance().getDevice(m_serial);
    if (!device) return QString();
    const QImage screenshot = device->captureFrame();
    if (screenshot.isNull()) return QString();

    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (!buffer.open(QIODevice::WriteOnly) || !screenshot.save(&buffer, "PNG"))
        return QString();
    return QStringLiteral("data:image/png;base64,") + QString::fromLatin1(bytes.toBase64());
}

QString NativeMapperWindow::profilePath(const QString &name) const {
    if (!QRegularExpression(QStringLiteral("^[\\p{L}\\p{N} _.-]{1,75}$"))
             .match(name).hasMatch() || name == "." || name == "..") return QString();
    const QString folder = currentMapDirectory();
    if (!QDir().mkpath(folder)) return QString();
    return QDir(folder).filePath(name + QStringLiteral(".json"));
}

QString NativeMapperWindow::saveProfile(const QString &json,
                                        const QString &requestedName) {
    const QString name = requestedName.isEmpty()
        ? m_profiles->currentText().trimmed()
        : requestedName.trimmed();
    const QString path = profilePath(name);
    if (path.isEmpty()) return QStringLiteral("ERRO: nome de perfil inválido");

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()
        || !doc.object().value(QStringLiteral("keyMapNodes")).isArray()) {
        return QStringLiteral("ERRO: estrutura JSON inválida");
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return QStringLiteral("ERRO: ") + file.errorString();
    const QByteArray bytes = doc.toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        return QStringLiteral("ERRO: não foi possível salvar o perfil");
    }
    m_currentName = name;
    m_status->setText(QStringLiteral("Salvo: ") + name);
    emit profileSaved(QString::fromUtf8(bytes), path);
    refreshProfiles();
    return QStringLiteral("OK: ") + path;
}

void NativeMapperWindow::refreshProfiles() {
    const QString selected = m_currentName.isEmpty() ? m_profiles->currentText() : m_currentName;
    const QSignalBlocker blocker(m_profiles);
    m_profiles->clear();
    QDir dir(currentMapDirectory());
    if (!dir.exists()) dir.mkpath(QStringLiteral("."));
    for (const QFileInfo &item : dir.entryInfoList({QStringLiteral("*.json")},
                                                    QDir::Files, QDir::Name)) {
        m_profiles->addItem(item.completeBaseName(), item.filePath());
    }
    const int idx = m_profiles->findText(selected);
    if (idx >= 0) m_profiles->setCurrentIndex(idx);
    else m_profiles->setCurrentIndex(-1);
    if (!m_view->url().isEmpty() && m_view->url().isValid() && idx >= 0)
        loadSelectedProfile();
}

void NativeMapperWindow::loadSelectedProfile() {
    const QString path = m_profiles->currentData().toString();
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) return;
    m_currentName = m_profiles->currentText();
    const QString json = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    const QString script = QStringLiteral("window.mapperInitialProfile=") + json
                         + QStringLiteral("; if(window.mapperReady) window.mapperLoadProfile(window.mapperInitialProfile);");
    m_view->page()->runJavaScript(script);
}

void NativeMapperWindow::createProfile() {
    bool accepted = false;
    const QString name = QInputDialog::getText(this, QStringLiteral("Novo perfil"),
                                               QStringLiteral("Nome do jogo:"),
                                               QLineEdit::Normal, QString(), &accepted).trimmed();
    if (!accepted || name.isEmpty()) return;
    const QString path = profilePath(name);
    if (path.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Nome inválido"),
                             QStringLiteral("Use letras, números, espaços, hífen e sublinhado."));
        return;
    }
    if (QFileInfo::exists(path)) {
        QMessageBox::information(this, QStringLiteral("Perfil existente"),
                                 QStringLiteral("Esse nome já está em uso."));
        return;
    }
    m_currentName = name;
    {
        const QSignalBlocker blocker(m_profiles);
        m_profiles->addItem(name, path);
        m_profiles->setCurrentIndex(m_profiles->count() - 1);
    }
    m_view->page()->runJavaScript(
        QStringLiteral("window.mapperInitialProfile=null; if(window.nodeManager) window.nodeManager.clearAllNodes();"));
    m_status->setText(QStringLiteral("Novo perfil: ") + name);
}
