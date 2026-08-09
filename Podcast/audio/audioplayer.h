#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QIODevice>

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    explicit AudioPlayer(QObject *parent = nullptr);
    ~AudioPlayer();

    void playChunk(const QByteArray &data);
    bool isReady() const;

private:
    QAudioSink *m_audioSink = nullptr;
    QIODevice *m_ioDevice = nullptr;
    QAudioFormat m_format;
};

#endif // AUDIOPLAYER_H