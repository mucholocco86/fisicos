#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QThread>
#include <QByteArray>
#include <QProcess>
#include <QPointer>
#include <QVector>

class QAudioSink;
class QAudioOutput;
class QIODevice;
class QTcpSocket;
class AudioOutput : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutput(QObject *parent = nullptr);
    ~AudioOutput();

    bool start(const QString& serial, int port);
    void stop();
    void installonly(const QString& serial, int port);

private:
    bool runSndcpyProcess(const QString& serial, int port, bool wait = true);
    void startAudioOutput();
    void stopAudioOutput();
    void startRecvData(int port);
    void stopRecvData();

signals:
    void connectTo(int port);

private:
    QPointer<QIODevice> m_outputDevice;
    QPointer<QTcpSocket> m_audioSocket;
    QProcess m_sndcpy;
    bool m_running = false;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QAudioOutput* m_audioOutput = nullptr;
#else
    QAudioSink *m_audioSink = nullptr;
    bool m_convertPcmToFloat = false;
    QByteArray m_pcmRemainder;
#endif
};

#endif // AUDIOOUTPUT_H
