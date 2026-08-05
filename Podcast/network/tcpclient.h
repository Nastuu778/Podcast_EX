#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    bool isConnected() const;

    void sendTextMessage(const QString &message);  // НОВОЕ

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &error);
    void textMessageReceived(const QString &message);  // НОВОЕ

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onReadyRead();  // НОВОЕ

private:
    QTcpSocket *m_socket;
};

#endif // TCPCLIENT_H