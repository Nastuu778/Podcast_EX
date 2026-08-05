#include "tcpserver.h"
#include <QDebug>
#include <QDataStream>

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
{
}

TcpServer::~TcpServer()
{
    stop();
}

bool TcpServer::start(quint16 port)
{
    if (listen(QHostAddress::Any, port)) {
        qDebug() << "Server started on port" << port;
        return true;
    } else {
        qDebug() << "Failed to start server:" << errorString();
        return false;
    }
}

void TcpServer::stop()
{
    for (QTcpSocket *client : m_clients) {
        client->close();
    }
    m_clients.clear();
    QTcpServer::close();
    qDebug() << "Server stopped";
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);

    m_clients.append(client);

    qDebug() << "New client connected:" << client->peerAddress().toString();

    // Подключаем сигнал readyRead для получения данных от клиента
    connect(client, &QTcpSocket::readyRead, this, &TcpServer::onClientReadyRead);

    connect(client, &QTcpSocket::disconnected, this, [this, client]() {
        m_clients.removeOne(client);
        qDebug() << "Client disconnected:" << client->peerAddress().toString();
        emit clientDisconnected(client);
    });

    emit clientConnected(client);
}

void TcpServer::onClientReadyRead()
{
    QTcpSocket *sender = qobject_cast<QTcpSocket*>(QObject::sender());
    if (!sender) return;

    QByteArray data = sender->readAll();

    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_6_0);

    QString message;
    stream >> message;

    if (!message.isEmpty()) {
        qDebug() << "Received from client:" << message;
        emit messageReceived(sender, message);

        // Рассылаем всем остальным клиентам
        broadcastMessage(message, sender);
    }
}

void TcpServer::broadcastMessage(const QString &message, QTcpSocket *sender)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << message;

    for (QTcpSocket *client : m_clients) {
        if (client != sender && client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}