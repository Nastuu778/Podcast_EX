#ifndef UDPMANAGER_H
#define UDPMANAGER_H

#include <QObject>
#include <QUdpSocket>

class UdpManager : public QObject
{
    Q_OBJECT

public:
    explicit UdpManager(QObject *parent = nullptr);
    ~UdpManager();

    bool bind();
    quint16 localPort() const;
    bool isBound() const;

    void setServerAddress(const QString &host, quint16 audioPort);
    void sendAudio(const QString &senderName, const QByteArray &audioData);

signals:
    void audioReceived(const QByteArray &data, const QString &senderName);

private:
    QUdpSocket *m_socket;
    QString m_serverHost;
    quint16 m_serverAudioPort = 0;
};

#endif // UDPMANAGER_H