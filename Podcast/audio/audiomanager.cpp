#include "audiomanager.h"
#include <QDebug>
#include <cstring>

AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
{
    // Формат аудио, оптимальный для речи:
    // 16 кГц, моно, 16 бит
    m_format.setSampleRate(16000);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);
}

AudioManager::~AudioManager()
{
    stopCapture();
}

QStringList AudioManager::availableMicrophones() const
{
    QStringList result;
    const QList<QAudioDevice> devices = QMediaDevices::audioInputs();
    for (const QAudioDevice &device : devices) {
        result.append(device.description());
    }
    return result;
}

bool AudioManager::startCapture(const QString &deviceName)
{
    if (m_capturing) {
        stopCapture();
    }

    // Ищем устройство по имени
    QAudioDevice selectedDevice;
    const QList<QAudioDevice> devices = QMediaDevices::audioInputs();
    for (const QAudioDevice &device : devices) {
        if (device.description() == deviceName) {
            selectedDevice = device;
            break;
        }
    }

    // Если не нашли - берём устройство по умолчанию
    if (selectedDevice.isNull()) {
        selectedDevice = QMediaDevices::defaultAudioInput();
    }

    // Проверяем поддержку формата
    if (!selectedDevice.isFormatSupported(m_format)) {
        emit captureError("Формат аудио не поддерживается этим устройством");
        return false;
    }

    // Создаём источник звука и запускаем захват
    m_audioSource = new QAudioSource(selectedDevice, m_format, this);
    m_audioIODevice = m_audioSource->start();

    if (!m_audioIODevice) {
        emit captureError("Не удалось запустить захват звука");
        return false;
    }

    // Читаем данные по мере поступления
    connect(m_audioIODevice, &QIODevice::readyRead, this, [this]() {
        if (!m_audioIODevice) return;

        QByteArray data = m_audioIODevice->readAll();
        if (data.isEmpty()) return;

        emit audioDataReady(data);

        // === БЕЗОПАСНЫЙ расчёт уровня громкости ===
        // Используем 32-битный тип, чтобы избежать переполнения
        // при максимальной громкости (значение -32768)
        qint32 maxVal = 0;
        const int sampleCount = data.size() / 2;
        const char *raw = data.constData();

        for (int i = 0; i < sampleCount; ++i) {
            // Безопасное чтение сэмпла (без проблем с выравниванием)
            qint16 sample;
            std::memcpy(&sample, raw + (i * 2), sizeof(qint16));

            // Модуль в 32 бита: -32768 -> 32768 (помещается!)
            const qint32 absVal = (sample < 0) ? -static_cast<qint32>(sample)
                                               : static_cast<qint32>(sample);
            if (absVal > maxVal) {
                maxVal = absVal;
            }
        }

        // Ограничиваем уровень диапазоном 0-100
        int level = static_cast<int>((maxVal * 100) / 32767);
        if (level > 100) level = 100;

        emit audioLevelChanged(level);
    });

    m_capturing = true;
    qDebug() << "Audio capture started:" << deviceName;
    return true;
}

void AudioManager::stopCapture()
{
    if (m_audioSource) {
        m_audioSource->stop();
        delete m_audioSource;
        m_audioSource = nullptr;
        m_audioIODevice = nullptr;
    }
    m_capturing = false;
    emit audioLevelChanged(0);
    qDebug() << "Audio capture stopped";
}

bool AudioManager::isCapturing() const
{
    return m_capturing;
}