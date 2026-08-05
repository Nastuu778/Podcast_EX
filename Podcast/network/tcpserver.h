#ifndef TCPSERVER_H
#define TCPSERVER_H

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

    QList<QTcpSocket*> m_clients;
    QMap<QTcpSocket*, QByteArray> m_clientBuffers;  // НОВОЕ - буферы для каждого клиента
    QStringList m_messageHistory;
    static const int MAX_HISTORY_SIZE = 15;
};

#endif // TCPSERVER_H