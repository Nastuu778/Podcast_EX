#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QAudioSource>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QIODevice>

class AudioManager : public QObject
{
    Q_OBJECT

public:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager();

    QStringList availableMicrophones() const;  // Список доступных микрофонов
    bool startCapture(const QString &deviceName);  // Начать захват
    void stopCapture();  // Остановить захват
    bool isCapturing() const;

signals:
    void audioDataReady(const QByteArray &data);  // Готовые данные для отправки
    void audioLevelChanged(int level);  // Уровень звука (0-100) для индикатора
    void captureError(const QString &error);

private:
    QAudioSource *m_audioSource = nullptr;
    QIODevice *m_audioIODevice = nullptr;
    QAudioFormat m_format;
    bool m_capturing = false;
};

#endif // AUDIOMANAGER_H