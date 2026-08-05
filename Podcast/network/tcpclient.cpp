#include "tcpclient.h"
#include <QDebug>
#include <QDataStream>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &TcpClient::onError);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);  // НОВОЕ
}

TcpClient::~TcpClient()
{
    disconnectFromServer();
}

void TcpClient::connectToServer(const QString &host, quint16 port)
{
    m_socket->connectToHost(host, port);
}

void TcpClient::disconnectFromServer()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

bool TcpClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::sendTextMessage(const QString &message)
{
    if (!isConnected()) return;

    // Используем QDataStream для надёжной передачи
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);

    stream << message;  // Записываем сообщение в поток

    m_socket->write(data);
    m_socket->flush();

    qDebug() << "Sent message:" << message;
}

void TcpClient::onConnected()
{
    qDebug() << "Connected to server";
    emit connected();
}

void TcpClient::onDisconnected()
{
    qDebug() << "Disconnected from server";
    emit disconnected();
}

void TcpClient::onError(QAbstractSocket::SocketError error)
{
    QString errorMsg = m_socket->errorString();
    qDebug() << "Socket error:" << errorMsg;
    emit errorOccurred(errorMsg);
}

void TcpClient::onReadyRead()
{
    // Читаем все доступные данные
    QByteArray data = m_socket->readAll();

    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_6_0);

    QString message;
    stream >> message;  // Извлекаем сообщение из потока

    if (!message.isEmpty()) {
        qDebug() << "Received message:" << message;
        emit textMessageReceived(message);
    }
}