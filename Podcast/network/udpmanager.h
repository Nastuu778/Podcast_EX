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

    bool bind();  // Создать UDP-сокет на свободном порту
    quint16 localPort() const;  // Наш UDP-порт
    bool isBound() const;

signals:
    void audioReceived(const QByteArray &data, const QString &senderName);  // На будущее

private:
    QUdpSocket *m_socket;
};

#endif // UDPMANAGER_H