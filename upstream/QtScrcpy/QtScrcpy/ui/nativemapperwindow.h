#pragma once
#include <QDialog>
#include <QPointer>
#include <QString>
#include <QWebEngineUrlSchemeHandler>

class QComboBox;
class QLabel;
class QWebEngineView;
class QWebChannel;
class QWebEngineUrlRequestJob;

// Scheme interno seguro para servir os arquivos originais offline, sem navegador.
void registerEmbeddedMapperScheme();

class MapperAssetHandler final : public QWebEngineUrlSchemeHandler {
    Q_OBJECT
public:
    explicit MapperAssetHandler(const QString &assetRoot, QObject *parent = nullptr);
    void requestStarted(QWebEngineUrlRequestJob *request) override;
private:
    QString m_root;
};

class NativeMapperWindow final : public QDialog {
    Q_OBJECT
public:
    explicit NativeMapperWindow(const QString &deviceSerial);

    // Métodos de ponte: mantêm o editor ScrcpyKeyMapper original,
    // porém salvam os perfis diretamente na pasta usada pelo QtScrcpy.
    Q_INVOKABLE QString saveProfile(const QString &json, const QString &requestedName);
    Q_INVOKABLE QString screenData() const;
    Q_INVOKABLE QString activeProfileName() const { return m_profiles ? m_currentName : QString(); }

signals:
    void profileSaved(const QString &json, const QString &filePath);

private:
    void refreshProfiles();
    void loadSelectedProfile();
    void createProfile();
    QString profilePath(const QString &name) const;

    QString m_serial;
    QWebEngineView *m_view = nullptr;
    QComboBox *m_profiles = nullptr;
    QLabel *m_status = nullptr;
    QString m_currentName;
};
