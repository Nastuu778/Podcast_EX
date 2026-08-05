#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QGroupBox>
#include <QSplitter>
#include "network/tcpclient.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &username,
                        const QString &host,
                        quint16 port,
                        int role,  // 0 = Speaker, 1 = Listener
                        QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onClientError(const QString &error);
    void onSendMessageClicked();
    void onTextMessageReceived(const QString &message);
    void updateParticipantList(const QString &data);

private:
    void setupUI();
    void updateStatus(const QString &status);
    void appendChatMessage(const QString &message);
    void sendRoleToServer();

    TcpClient *m_client;
    QString m_username;
    QString m_host;
    quint16 m_port;
    int m_role;  // 0 = Speaker, 1 = Listener

    // UI элементы
    QLabel *m_statusLabel;
    QLabel *m_roleLabel;
    QListWidget *m_speakersList;
    QListWidget *m_listenersList;
    QTextEdit *m_chatDisplay;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;
    QPushButton *m_disconnectButton;
};

#endif // MAINWINDOW_H