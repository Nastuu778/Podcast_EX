#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "network/tcpclient.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectButtonClicked();
    void onClientConnected();
    void onClientDisconnected();
    void onClientError(const QString &error);
    void onSendMessageClicked();  // НОВОЕ
    void onTextMessageReceived(const QString &message);  // НОВОЕ

private:
    void setupUI();
    void updateStatus(const QString &status);
    void appendChatMessage(const QString &message);  // НОВОЕ

    TcpClient *m_client;
    QLabel *m_statusLabel;
    QLineEdit *m_hostInput;
    QLineEdit *m_portInput;
    QPushButton *m_connectButton;

    // НОВЫЕ элементы для чата
    QTextEdit *m_chatDisplay;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;
};

#endif // MAINWINDOW_H