#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QHostAddress>
#include <QPair>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent = nullptr);
    ~TcpServer();

    bool start(quint16 port);
    void stop();

signals:
    void clientConnected(QTcpSocket *client);
    void clientDisconnected(QTcpSocket *client);
    void messageReceived(QTcpSocket *sender, const QString &message);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientReadyRead();

private:
    void broadcastMessage(const QString &message, QTcpSocket *sender);
    void sendHistoryToClient(QTcpSocket *client);
    void addMessageToHistory(const QString &message);
    void sendParticipantListToClient(QTcpSocket *client);  // НОВОЕ
    void sendParticipantListToAll();  // НОВОЕ
    void handleUdpPort(QTcpSocket *client, const QString &message);
    int countClientsByRole(const QString &role);
    QString getClientUsername(QTcpSocket *client);  // НОВОЕ

    QMap<QTcpSocket*, QPair<QHostAddress, quint16>> m_clientUdpAddresses;
    QList<QTcpSocket*> m_clients;
    QMap<QTcpSocket*, QByteArray> m_clientBuffers;
    QMap<QTcpSocket*, QString> m_clientUsernames;  // НОВОЕ
    QMap<QTcpSocket*, QString> m_clientRoles;  // НОВОЕ
    QStringList m_messageHistory;
    static const int MAX_HISTORY_SIZE = 15;
    static const int MAX_SPEAKERS = 2;
    static const int MAX_LISTENERS = 6;
};

#endif // TCPSERVER_H