#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QGroupBox>
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
    void onSendMessageClicked();
    void onTextMessageReceived(const QString &message);

private:
    void setupUI();
    void updateStatus(const QString &status);
    void appendChatMessage(const QString &message);
    bool validateUsername();  // НОВОЕ

    TcpClient *m_client;
    QLabel *m_statusLabel;
    QLineEdit *m_hostInput;
    QLineEdit *m_portInput;
    QLineEdit *m_usernameInput;  // НОВОЕ
    QPushButton *m_connectButton;

    QTextEdit *m_chatDisplay;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;

    QString m_username;  // НОВОЕ - храним имя пользователя
};

#endif // MAINWINDOW_H