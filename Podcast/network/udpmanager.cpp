#include "udpmanager.h"
#include <QDebug>

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