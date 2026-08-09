#include "audioplayer.h"
#include <QDebug>

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
{
    // Тот же формат, что и при захвате: 16 кГц, моно, 16 бит
    m_format.setSampleRate(16000);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice device = QMediaDevices::defaultAudioOutput();

    if (!device.isFormatSupported(m_format)) {
        qDebug() << "Audio output format not supported!";
        return;
    }

    m_audioSink = new QAudioSink(device, m_format, this);
    m_ioDevice = m_audioSink->start();

    if (m_ioDevice) {
        qDebug() << "Audio player ready";
    }
}

AudioPlayer::~AudioPlayer()
{
    if (m_audioSink) {
        m_audioSink->stop();
    }
}

void AudioPlayer::playChunk(const QByteArray &data)
{
    if (!m_ioDevice || !m_audioSink) return;

    // Пишем в буфер столько, сколько влезает.
    // Если буфер полон - отбрасываем (для живого звука лучше
    // потерять кусочек, чем накапливать задержку)
    qint64 written = 0;
    while (written < data.size()) {
        qint64 free = m_audioSink->bytesFree();
        if (free == 0) break;

        qint64 toWrite = qMin(free, static_cast<qint64>(data.size() - written));
        qint64 w = m_ioDevice->write(data.constData() + written, toWrite);
        if (w <= 0) break;
        written += w;
    }
}

bool AudioPlayer::isReady() const
{
    return m_ioDevice != nullptr;
}

void AudioPlayer::setVolume(qreal volume)
{
    if (m_audioSink) {
        m_audioSink->setVolume(volume);
    }
}