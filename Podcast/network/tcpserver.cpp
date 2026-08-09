#include "tcpserver.h"
#include <QDebug>
#include <QDataStream>

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
    , m_audioSocket(nullptr)
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
        // UDP-сокет для приёма аудио на порту (port + 1)
        m_audioSocket = new QUdpSocket(this);
        if (m_audioSocket->bind(QHostAddress::Any, port + 1)) {
            qDebug() << "Audio UDP socket bound to port" << (port + 1);
            connect(m_audioSocket, &QUdpSocket::readyRead, this, &TcpServer::onAudioReadyRead);
        } else {
            qDebug() << "Failed to bind audio UDP socket:" << m_audioSocket->errorString();
        }
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
    m_clientUsernames.clear();
    m_clientRoles.clear();
    m_clientUdpAddresses.clear();
    m_messageHistory.clear();
    QTcpServer::close();
    if (m_audioSocket) {
        m_audioSocket->close();
        delete m_audioSocket;
        m_audioSocket = nullptr;
    }
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

    // Затем отправляем список участников
    sendParticipantListToClient(client);

    connect(client, &QTcpSocket::disconnected, this, [this, client]() {
        // Получаем имя клиента ПЕРЕД удалением
        QString username = getClientUsername(client);

        m_clients.removeOne(client);
        m_clientBuffers.remove(client);
        m_clientUsernames.remove(client);
        m_clientRoles.remove(client);
        m_clientUdpAddresses.remove(client);

        qDebug() << "Client disconnected:" << client->peerAddress().toString();

        // Создаём сообщение об отключении
        if (!username.isEmpty()) {
            QString disconnectMessage = username + " отключился от сервера";
            addMessageToHistory(disconnectMessage);
            broadcastMessage(disconnectMessage, nullptr);

            // Обновляем список участников для всех
            sendParticipantListToAll();
        }

        emit clientDisconnected(client);
    });

    emit clientConnected(client);
}

QString TcpServer::getClientUsername(QTcpSocket *client)
{
    return m_clientUsernames.value(client, "");
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

void TcpServer::handleUdpPort(QTcpSocket *client, const QString &message)
{
    bool ok = false;
    quint16 port = message.mid(9).toUShort(&ok);  // Убираем "/udpport:"

    if (ok) {
        // Берём адрес клиента из TCP-соединения
        QHostAddress addr = client->peerAddress();

        // Преобразуем IPv4-mapped IPv6 (::ffff:127.0.0.1) в обычный IPv4
        // Работаем через строку - это надёжно во всех версиях Qt
        QString addrStr = addr.toString();
        if (addrStr.startsWith("::ffff:")) {
            addr = QHostAddress(addrStr.mid(7));  // Убираем "::ffff:"
        }

        m_clientUdpAddresses[client] = qMakePair(addr, port);
        qDebug() << "Registered UDP for client:" << addr.toString() << ":" << port;
    }
}

int TcpServer::countClientsByRole(const QString &role)
{
    int count = 0;
    for (const QString &r : m_clientRoles.values()) {
        if (r == role) {
            count++;
        }
    }
    return count;
}

void TcpServer::sendParticipantListToClient(QTcpSocket *client)
{
    // Собираем списки спикеров и слушателей
    QStringList speakers;
    QStringList listeners;

    for (QTcpSocket *c : m_clients) {
        QString username = m_clientUsernames.value(c);
        QString role = m_clientRoles.value(c);

        if (role == "speaker") {
            speakers.append(username);
        } else if (role == "listener") {
            listeners.append(username);
        }
    }

    // Формируем сообщение
    QString message = "/participants:speakers:" + speakers.join(',') +
                      ";listeners:" + listeners.join(',');

    // Отправляем клиенту
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << message;

    client->write(data);
    client->flush();

    qDebug() << "Sent participant list to client";
}

void TcpServer::sendParticipantListToAll()
{
    for (QTcpSocket *client : m_clients) {
        sendParticipantListToClient(client);
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

                // Проверяем, это UDP-порт клиента?
                if (message.startsWith("/udpport:")) {
                    handleUdpPort(sender, message);
                    continue;  // Это служебная команда, не рассылаем
                }

                // Проверяем, это команда /join:?
                if (message.startsWith("/join:")) {
                    // Формат: /join:username:role
                    QStringList parts = message.mid(6).split(':');
                    if (parts.size() == 2) {
                        QString username = parts[0];
                        QString role = parts[1];

                        qDebug() << "Client joined:" << username << "as" << role;

                        // === ПРОВЕРКА ЛИМИТОВ ===
                        if (role == "speaker") {
                            int speakerCount = countClientsByRole("speaker");
                            if (speakerCount >= MAX_SPEAKERS) {
                                // Лимит спикеров достигнут
                                QString errorMsg = "/error:Достигнут лимит спикеров (максимум 2). Попробуйте подключиться как слушатель.";

                                QByteArray data;
                                QDataStream stream(&data, QIODevice::WriteOnly);
                                stream.setVersion(QDataStream::Qt_6_0);
                                stream << errorMsg;

                                sender->write(data);
                                sender->flush();

                                qDebug() << "Rejected speaker" << username << "- limit reached";
                                continue;
                            }
                        } else if (role == "listener") {
                            int listenerCount = countClientsByRole("listener");
                            if (listenerCount >= MAX_LISTENERS) {
                                // Лимит слушателей достигнут
                                QString errorMsg = "/error:Достигнут лимит слушателей (максимум 6). Попробуйте позже.";

                                QByteArray data;
                                QDataStream stream(&data, QIODevice::WriteOnly);
                                stream.setVersion(QDataStream::Qt_6_0);
                                stream << errorMsg;

                                sender->write(data);
                                sender->flush();

                                qDebug() << "Rejected listener" << username << "- limit reached";
                                continue;
                            }
                        }

                        // === Если лимит не достигнут - регистрируем клиента ===
                        m_clientUsernames[sender] = username;
                        m_clientRoles[sender] = role;

                        // Создаём системное сообщение
                        QString systemMessage = username + " подключился к серверу как " +
                                                (role == "speaker" ? "спикер" : "слушатель");

                        // Добавляем в историю
                        addMessageToHistory(systemMessage);

                        // Отправляем всем клиентам (включая отправителя)
                        broadcastMessage(systemMessage, nullptr);

                        // Обновляем список участников для всех
                        sendParticipantListToAll();
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

void TcpServer::onAudioReadyRead()
{
    while (m_audioSocket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(static_cast<int>(m_audioSocket->pendingDatagramSize()));

        QHostAddress senderAddr;
        quint16 senderPort = 0;
        m_audioSocket->readDatagram(data.data(), data.size(), &senderAddr, &senderPort);

        // Извлекаем имя отправителя и аудио
        QDataStream stream(data);
        stream.setVersion(QDataStream::Qt_6_0);

        QString senderName;
        QByteArray audio;
        stream >> senderName >> audio;

        qDebug() << "Received audio from" << senderName
                 << "| size:" << audio.size()
                 << "| from" << senderAddr.toString() << ":" << senderPort;

        // TODO (Этап 3): пересылка всем, кроме отправителя
    }
}