#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

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
    void messageReceived(QTcpSocket *sender, const QString &message);  // НОВОЕ

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientReadyRead();  // НОВОЕ

private:
    void broadcastMessage(const QString &message, QTcpSocket *sender);  // НОВОЕ

    QList<QTcpSocket*> m_clients;
};

#endif // TCPSERVER_H