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
    m_clientBuffers.clear();
    m_messageHistory.clear();
    QTcpServer::close();
    qDebug() << "Server stopped";
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);

    m_clients.append(client);
    m_clientBuffers[client] = QByteArray();

    qDebug() << "New client connected:" << client->peerAddress().toString();

    connect(client, &QTcpSocket::readyRead, this, &TcpServer::onClientReadyRead);

    // Сначала отправляем историю
    sendHistoryToClient(client);
    // Затем создаём системное сообщение о подключении
    // Его нужно получить от клиента (username)
    // Поэтому пока просто добавляем клиента в список

    connect(client, &QTcpSocket::disconnected, this, [this, client]() {
        m_clients.removeOne(client);
        m_clientBuffers.remove(client);
        qDebug() << "Client disconnected:" << client->peerAddress().toString();
        emit clientDisconnected(client);
    });

    emit clientConnected(client);
}

void TcpServer::sendHistoryToClient(QTcpSocket *client)
{
    if (m_messageHistory.isEmpty()) {
        qDebug() << "No message history to send";
        return;
    }

    for (const QString &message : m_messageHistory) {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_6_0);
        stream << message;

        client->write(data);
        client->flush();
    }

    qDebug() << "Sent" << m_messageHistory.size() << "messages from history to new client";
}

void TcpServer::addMessageToHistory(const QString &message)
{
    m_messageHistory.append(message);

    if (m_messageHistory.size() > MAX_HISTORY_SIZE) {
        m_messageHistory.removeFirst();
    }
}

void TcpServer::onClientReadyRead()
{
    QTcpSocket *sender = qobject_cast<QTcpSocket*>(QObject::sender());
    if (!sender || !m_clientBuffers.contains(sender)) return;

    m_clientBuffers[sender].append(sender->readAll());

    while (m_clientBuffers[sender].size() > 0) {
        QDataStream stream(m_clientBuffers[sender]);
        stream.setVersion(QDataStream::Qt_6_0);

        QString message;
        stream >> message;

        if (stream.status() == QDataStream::Ok) {
            int bytesRead = m_clientBuffers[sender].size() - stream.device()->bytesAvailable();
            m_clientBuffers[sender].remove(0, bytesRead);

            if (!message.isEmpty()) {
                qDebug() << "Received from client:" << message;

                // Проверяем, это команда /join:?
                if (message.startsWith("/join:")) {
                    // Формат: /join:username:role
                    QStringList parts = message.mid(6).split(':');
                    if (parts.size() == 2) {
                        QString username = parts[0];
                        QString role = parts[1];

                        qDebug() << "Client joined:" << username << "as" << role;

                        // Создаём системное сообщение
                        QString systemMessage = username + " подключился к серверу как " +
                                                (role == "speaker" ? "спикер" : "слушатель");

                        // Добавляем в историю
                        addMessageToHistory(systemMessage);

                        // Отправляем всем клиентам (включая отправителя)
                        broadcastMessage(systemMessage, nullptr);
                    }
                } else {
                    // Обычное сообщение чата
                    emit messageReceived(sender, message);
                    addMessageToHistory(message);
                    broadcastMessage(message, sender);
                }
            }
        } else {
            break;
        }
    }
}

void TcpServer::broadcastMessage(const QString &message, QTcpSocket *sender)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << message;

    for (QTcpSocket *client : m_clients) {
        // Если sender == nullptr, отправляем ВСЕМ (системное сообщение)
        // Иначе отправляем всем КРОМЕ отправителя
        if ((sender == nullptr || client != sender) &&
            client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}