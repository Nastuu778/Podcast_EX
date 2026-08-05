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
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
}

TcpClient::~TcpClient()
{
    disconnectFromServer();
}

void TcpClient::connectToServer(const QString &host, quint16 port)
{
    m_buffer.clear();  // Очищаем буфер при новом подключении
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

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);

    stream << message;

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
    // Добавляем новые данные в буфер
    m_buffer.append(m_socket->readAll());

    // Пытаемся извлечь все сообщения из буфера
    while (m_buffer.size() > 0) {
        QDataStream stream(m_buffer);
        stream.setVersion(QDataStream::Qt_6_0);

        QString message;
        stream >> message;

        // Проверяем, удалось ли прочитать сообщение
        if (stream.status() == QDataStream::Ok) {
            // Вычисляем сколько байт было прочитано
            int bytesRead = m_buffer.size() - stream.device()->bytesAvailable();
            m_buffer.remove(0, bytesRead);  // Удаляем прочитанное из буфера

            if (!message.isEmpty()) {
                qDebug() << "Received message:" << message;
                emit textMessageReceived(message);
            }
        } else {
            // Данные неполные, ждём ещё
            break;
        }
    }
}

void TcpClient::sendUsername(const QString &username)
{
    if (!isConnected()) return;

    QString usernameMessage = QString("/username:%1").arg(username);

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << usernameMessage;

    m_socket->write(data);
    m_socket->flush();

    qDebug() << "Sent username:" << username;
}