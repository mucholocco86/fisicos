#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHostAddress>
#include <QMainWindow>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>
#include <QWebEngineDownloadRequest>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <QVBoxLayout>
#include <QWidget>

class LocalAssetServer final : public QObject {
public:
    explicit LocalAssetServer(QString root, QObject *parent = nullptr)
        : QObject(parent), m_root(QDir(root).canonicalPath()) {
        connect(&m_server, &QTcpServer::newConnection, this, [this]() {
            while (auto *socket = m_server.nextPendingConnection()) {
                connect(socket, &QTcpSocket::readyRead, socket, [this, socket]() {
                    if (socket->property("handled").toBool()) return;
                    if (!socket->canReadLine()) return;
                    const QByteArray first = socket->readLine().trimmed();
                    const QList<QByteArray> parts = first.split(' ');
                    socket->setProperty("handled", true);
                    QByteArray response;
                    QByteArray mime = "text/plain; charset=utf-8";
                    int code = 404;
                    if (parts.size() >= 2 && parts[0] == "GET") {
                        const QString relative = QUrl::fromPercentEncoding(parts[1].split('?').first());
                        QString name = relative == "/" ? "/index.html" : relative;
                        const QString absolute = QFileInfo(m_root + name).canonicalFilePath();
                        if (!absolute.isEmpty() && absolute.startsWith(m_root + QDir::separator())) {
                            QFile file(absolute);
                            if (file.open(QIODevice::ReadOnly)) {
                                response = file.readAll();
                                mime = QMimeDatabase().mimeTypeForFile(absolute).name().toUtf8();
                                if (absolute.endsWith(".js")) mime = "text/javascript";
                                if (absolute.endsWith(".css")) mime = "text/css";
                                code = 200;
                            }
                        }
                    }
                    const QByteArray status = code == 200 ? "200 OK" : "404 Not Found";
                    socket->write("HTTP/1.1 " + status + "\r\nContent-Type: " + mime
                        + "\r\nX-Content-Type-Options: nosniff\r\nCache-Control: no-cache\r\nContent-Length: "
                        + QByteArray::number(response.size()) + "\r\nConnection: close\r\n\r\n");
                    socket->write(response);
                    socket->disconnectFromHost();
                });
                connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
            }
        });
    }
    bool start() { return !m_root.isEmpty() && m_server.listen(QHostAddress::LocalHost, 0); }
    QUrl url() const { return QUrl(QString("http://127.0.0.1:%1/").arg(m_server.serverPort())); }
private:
    QString m_root;
    QTcpServer m_server;
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setApplicationName("ScrcpyKeyMapper");
    app.setOrganizationName("Wells");
    const QString webRoot = QCoreApplication::applicationDirPath() + "/editor";
    LocalAssetServer assets(webRoot);
    if (!assets.start()) {
        QMessageBox::critical(nullptr, "ScrcpyKeyMapper", "Não foi possível abrir os arquivos locais do editor.");
        return 1;
    }
    QMainWindow window;
    window.setWindowTitle("ScrcpyKeyMapper — Editor independente");
    window.setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    auto *view = new QWebEngineView(&window);
    window.setCentralWidget(view);
    window.resize(1180, 760);
    window.setMinimumSize(800, 540);
    auto *profile = view->page()->profile();
    QObject::connect(profile, &QWebEngineProfile::downloadRequested, &window,
        [&window](QWebEngineDownloadRequest *download) {
            const QString suggested = download->downloadFileName().isEmpty()
                ? "key-mapping-config.json" : download->downloadFileName();
            const QString target = QFileDialog::getSaveFileName(&window, "Salvar mapeamento JSON",
                QDir::home().filePath(suggested), "Arquivos JSON (*.json);;Todos os arquivos (*)");
            if (target.isEmpty()) { download->cancel(); return; }
            download->setDownloadDirectory(QFileInfo(target).absolutePath());
            download->setDownloadFileName(QFileInfo(target).fileName());
            download->accept();
        });
    QObject::connect(view, &QWebEngineView::loadFinished, &window, [&window](bool ok) {
        if (!ok) QMessageBox::warning(&window, "ScrcpyKeyMapper", "Falha ao carregar o editor local.");
    });
    view->load(assets.url());
    window.show();
    return app.exec();
}
