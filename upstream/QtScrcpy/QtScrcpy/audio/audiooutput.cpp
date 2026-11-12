#include <QAudioOutput>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMetaObject>
#include <QByteArray>
#include <QTimer>
#include <QtEndian>
#include <cstring>
#include <utility>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTime>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QAudioSink>
#include <QAudioDevice>
#include <QMediaDevices>
#endif

#include "audiooutput.h"

AudioOutput::AudioOutput(QObject *parent)
    : QObject(parent)
{
    m_running = false;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    m_audioOutput = nullptr;
#else
    m_audioSink = nullptr;
#endif
    connect(&m_sndcpy, &QProcess::readyReadStandardOutput, this, [this]() {
        qInfo() << QString("AudioOutput::") << QString(m_sndcpy.readAllStandardOutput());
    });
    connect(&m_sndcpy, &QProcess::readyReadStandardError, this, [this]() {
        qInfo() << QString("AudioOutput::") << QString(m_sndcpy.readAllStandardError());
    });
}

AudioOutput::~AudioOutput()
{
    stop();
    if (m_sndcpy.state() != QProcess::NotRunning) {
        m_sndcpy.kill();
        m_sndcpy.waitForFinished(3000);
    }
}

bool AudioOutput::start(const QString& serial, int port)
{
    // A failed previous attempt must not leave the sink or socket behind.
    stop();

    QElapsedTimer timeConsumeCount;
    timeConsumeCount.start();
    bool ret = runSndcpyProcess(serial, port);
    qInfo() << "AudioOutput::run sndcpy cost:" << timeConsumeCount.elapsed() << "milliseconds";
    if (!ret) {
        return ret;
    }

    startAudioOutput();
    if (!m_outputDevice) {
        qWarning() << "AudioOutput::sndcpy playback initialization failed; socket will not be started.";
        stopAudioOutput();
        return false;
    }
    startRecvData(port);

    m_running = true;
    return true;
}

void AudioOutput::stop()
{
    m_running = false;
    stopRecvData();
    stopAudioOutput();
}

void AudioOutput::installonly(const QString &serial, int port)
{
    runSndcpyProcess(serial, port, false);
}

bool AudioOutput::runSndcpyProcess(const QString &serial, int port, bool wait)
{
    if (QProcess::NotRunning != m_sndcpy.state()) {
        m_sndcpy.kill();
        m_sndcpy.waitForFinished(3000);
    }

#ifdef Q_OS_WIN32
    QStringList params{serial, QString::number(port)};
    const QString script = QDir(QCoreApplication::applicationDirPath()).filePath("sndcpy.bat");
    if (!QFileInfo::exists(script)) {
        qWarning() << "AudioOutput::missing sndcpy script:" << script;
        return false;
    }
    m_sndcpy.setWorkingDirectory(QCoreApplication::applicationDirPath());
    m_sndcpy.start(script, params);
#else
    QStringList params{"sndcpy.sh", serial, QString::number(port)};
    m_sndcpy.setWorkingDirectory(QCoreApplication::applicationDirPath());
    m_sndcpy.start("bash", params);
#endif

    if (!wait) {
        return true;
    }

    if (!m_sndcpy.waitForStarted()) {
        qWarning() << "AudioOutput::start sndcpy process failed";
        return false;
    }
    if (!m_sndcpy.waitForFinished()) {
        qWarning() << "AudioOutput::sndcpy script did not finish successfully:" << m_sndcpy.errorString();
        return false;
    }
    if (m_sndcpy.exitStatus() != QProcess::NormalExit || m_sndcpy.exitCode() != 0) {
        qWarning() << "AudioOutput::sndcpy script failed, exit code:" << m_sndcpy.exitCode();
        return false;
    }

    return true;
}

void AudioOutput::startAudioOutput()
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (m_audioOutput) {
        return;
    }

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());

    if (!info.isFormatSupported(format)) {
        qWarning() << "AudioOutput::audio format not supported, cannot play audio.";
        return;
    }

    m_audioOutput = new QAudioOutput(format, this);
    connect(m_audioOutput, &QAudioOutput::stateChanged, this, [](QAudio::State state) {
        qInfo() << "AudioOutput::audio state changed:" << state;
    });
    m_audioOutput->setBufferSize(48000*2*15/1000 * 20);
    m_outputDevice = m_audioOutput->start();
#else
    if (m_audioSink) {
        return;
    }

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);
    QAudioDevice defaultDevice = QMediaDevices::defaultAudioOutput();
    if (defaultDevice.isNull()) {
        qWarning() << "AudioOutput::Windows/Qt has no default audio output device.";
        return;
    }
    qInfo() << "AudioOutput::output device:" << defaultDevice.description()
            << "preferred format:" << defaultDevice.preferredFormat()
            << "sndcpy format:" << format;
    m_convertPcmToFloat = false;
    if (!defaultDevice.isFormatSupported(format)) {
        // Preserve 48 kHz/stereo. Convert 16-bit signed little-endian PCM to
        // float only when Windows/Qt refuses Int16, e.g. FxSound virtual output.
        QAudioFormat floatFormat = format;
        floatFormat.setSampleFormat(QAudioFormat::Float);
        if (!defaultDevice.isFormatSupported(floatFormat)) {
            qWarning() << "AudioOutput::output does not support 48 kHz stereo Int16 or Float PCM.";
            return;
        }
        format = floatFormat;
        m_convertPcmToFloat = true;
        qInfo() << "AudioOutput::converting sndcpy Int16 PCM to 48 kHz stereo Float for output device.";
    }
    m_pcmRemainder.clear();
    m_pendingAudio.clear();
    m_audioFrameBytes = format.bytesPerFrame();
    m_maxPendingAudioBytes = format.bytesForDuration(100000);
    m_audioSink = new QAudioSink(defaultDevice, format, this);
    // Bound queued playback to ~100 ms instead of relying on the Qt6 backend
    // default. Calculate from the negotiated format (Int16 or Float), so the
    // FxSound-compatible Float path gets the correct byte capacity.
    const qint64 targetBufferBytes = format.bytesForDuration(100000);
    m_audioSink->setBufferSize(static_cast<qsizetype>(targetBufferBytes));
    qInfo() << "AudioOutput::requested 100 ms audio buffer:" << targetBufferBytes << "bytes";
    m_outputDevice = m_audioSink->start();
    qInfo() << "AudioOutput::actual Qt6 audio buffer:" << m_audioSink->bufferSize() << "bytes";
    if (!m_outputDevice) {
        qWarning() << "AudioOutput::audio output device not available, cannot play audio.";
        delete m_audioSink;
        m_audioSink = nullptr;
        return;
    }
    m_audioDrainTimer = new QTimer(this);
    m_audioDrainTimer->setInterval(10);
    connect(m_audioDrainTimer, &QTimer::timeout, this, &AudioOutput::drainAudio);
    m_audioDrainTimer->start();
    connect(m_audioSink, &QAudioSink::stateChanged, this, [this](QAudio::State state) {
        if (m_audioSink && state == QAudio::StoppedState && m_audioSink->error() != QAudio::NoError) {
            qWarning() << "AudioOutput::sink error:" << m_audioSink->error();
        }
    });
#endif
}

void AudioOutput::stopAudioOutput()
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    if (m_audioOutput) {
        m_audioOutput->stop();
        delete m_audioOutput;
        m_audioOutput = nullptr;
    }
#else
    if (m_audioDrainTimer) {
        m_audioDrainTimer->stop();
        delete m_audioDrainTimer;
        m_audioDrainTimer = nullptr;
    }
    m_pendingAudio.clear();
    m_maxPendingAudioBytes = 0;
    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
    }
#endif
    m_outputDevice = nullptr;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    m_pcmRemainder.clear();
    m_convertPcmToFloat = false;
#endif
}

void AudioOutput::startRecvData(int port)
{
    stopRecvData();

    // The socket and QAudioSink share the main Qt thread. This prevents
    // cross-thread QIODevice writes and destruction races on reconnect.
    auto audioSocket = new QTcpSocket(this);
    m_audioSocket = audioSocket;
    connect(audioSocket, &QTcpSocket::disconnected, this, [this, audioSocket]() {
        if (m_audioSocket == audioSocket) {
            m_audioSocket = nullptr;
        }
        audioSocket->deleteLater();
    });
    connect(audioSocket, &QTcpSocket::errorOccurred, this, [this, audioSocket](QAbstractSocket::SocketError error) {
        qWarning() << "AudioOutput::sndcpy TCP error:" << error << audioSocket->errorString();
    });

    connect(audioSocket, &QTcpSocket::connected, this, []() {
        qInfo("AudioOutput::audio socket connect success");
    });
    connect(audioSocket, &QIODevice::readyRead, this, [this, audioSocket]() {
        QByteArray samples = audioSocket->readAll();
        if (!m_outputDevice || samples.isEmpty()) {
            return;
        }
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        // sndcpy sends 48 kHz, stereo Int16 PCM (4 bytes per sample frame).
        // TCP may split a frame across readyRead notifications.
        samples.prepend(m_pcmRemainder);
        const qsizetype alignedLength = samples.size() - (samples.size() % 4);
        m_pcmRemainder = samples.mid(alignedLength);
        samples.truncate(alignedLength);
        if (m_convertPcmToFloat) {
            const char *pcm = samples.constData();
            QByteArray floats(alignedLength * 2, '\0');
            for (qsizetype i = 0; i < alignedLength; i += 2) {
                const quint16 raw = quint16(quint8(pcm[i])) | (quint16(quint8(pcm[i + 1])) << 8);
                const qint16 value = static_cast<qint16>(raw);
                const float normalized = float(value) / 32768.0f;
                memcpy(floats.data() + i * 2, &normalized, sizeof(float));
            }
            samples = std::move(floats);
        }
        enqueueAudio(std::move(samples));
#else
        if (!samples.isEmpty()) {
            const qint64 written = m_outputDevice->write(samples);
            if (written < 0) {
                qWarning() << "AudioOutput::PCM write failed:" << m_outputDevice->errorString();
            } else if (written != samples.size()) {
                qWarning() << "AudioOutput::PCM output buffer full; dropped" << (samples.size() - written) << "bytes";
            }
        }
#endif
    });
    connect(audioSocket, &QTcpSocket::stateChanged, audioSocket, [](QAbstractSocket::SocketState state) {
        qInfo() << "AudioOutput::audio socket state changed:" << state;

    });
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    // Error reporting is handled by the parent-thread callback above.
#else
    connect(audioSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), audioSocket, [](QAbstractSocket::SocketError error) {
        qInfo() << "AudioOutput::audio socket error occurred:" << error;
    });
#endif

    audioSocket->connectToHost(QHostAddress::LocalHost, port);
}

void AudioOutput::stopRecvData()
{
    if (m_audioSocket) {
        QTcpSocket *socket = m_audioSocket;
        m_audioSocket = nullptr;
        socket->disconnect(this);
        socket->abort();
        socket->deleteLater();
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void AudioOutput::enqueueAudio(QByteArray samples)
{
    if (!m_audioSink || !m_outputDevice || samples.isEmpty()) {
        return;
    }
    m_pendingAudio.append(samples);
    // Bound latency: never keep more than 100 ms of PCM waiting in our queue.
    if (m_maxPendingAudioBytes > 0 && m_pendingAudio.size() > m_maxPendingAudioBytes) {
        const qsizetype excess = m_pendingAudio.size() - m_maxPendingAudioBytes;
        const qsizetype drop = ((excess + m_audioFrameBytes - 1) / m_audioFrameBytes) * m_audioFrameBytes;
        m_pendingAudio.remove(0, drop);
        qWarning() << "AudioOutput::pending PCM latency limit; dropped" << drop << "oldest bytes";
    }
    drainAudio();
}

void AudioOutput::drainAudio()
{
    if (!m_audioSink || !m_outputDevice || m_pendingAudio.isEmpty()) {
        return;
    }
    const qint64 freeBytes = m_audioSink->bytesFree();
    const qsizetype available = qMin(qsizetype(freeBytes), m_pendingAudio.size());
    const qsizetype aligned = available - (available % m_audioFrameBytes);
    if (aligned <= 0) {
        return;
    }
    const qint64 written = m_outputDevice->write(m_pendingAudio.constData(), aligned);
    if (written < 0) {
        qWarning() << "AudioOutput::PCM write failed:" << m_outputDevice->errorString();
        return;
    }
    if (written > 0) {
        // QAudioSink normally accepts complete frames. Keep the remainder if
        // it performs a partial write rather than silently losing the samples.
        m_pendingAudio.remove(0, written);
    }
}
#endif
