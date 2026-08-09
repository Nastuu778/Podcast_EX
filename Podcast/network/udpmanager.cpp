#include "udpmanager.h"
#include <QDebug>
#include <QDataStream>

UdpManager::UdpManager(QObject *parent)
    : QObject(parent)
    , m_socket(new QUdpSocket(this))
{
}

UdpManager::~UdpManager()
{
}

bool UdpManager::bind()
{
    if (m_socket->state() == QAbstractSocket::BoundState) {
        return true;
    }

    // Порт 0 = система сама выберет свободный порт
    bool ok = m_socket->bind(QHostAddress::Any, 0);

    if (ok) {
        qDebug() << "UDP socket bound to port" << m_socket->localPort();
    } else {
        qDebug() << "Failed to bind UDP socket:" << m_socket->errorString();
    }

    return ok;
}

quint16 UdpManager::localPort() const
{
    return m_socket->localPort();
}

bool UdpManager::isBound() const
{
    return m_socket->state() == QAbstractSocket::BoundState;
}

void UdpManager::setServerAddress(const QString &host, quint16 audioPort)
{
    m_serverHost = host;
    m_serverAudioPort = audioPort;
}

void UdpManager::sendAudio(const QString &senderName, const QByteArray &audioData)
{
    if (m_serverAudioPort == 0) return;

    // Формат пакета: имя отправителя + аудио-данные
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << senderName << audioData;

    m_socket->writeDatagram(data, QHostAddress(m_serverHost), m_serverAudioPort);
}