#include "tcpserver.h"
#include <QDebug>

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

    connect(client, &QTcpSocket::disconnected, this, [this, client]() {
        m_clients.removeOne(client);
        qDebug() << "Client disconnected:" << client->peerAddress().toString();
        emit clientDisconnected(client);
    });

    emit clientConnected(client);
}