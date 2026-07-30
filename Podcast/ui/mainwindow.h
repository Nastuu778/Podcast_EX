#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
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

private:
    void setupUI();
    void updateStatus(const QString &status);

    TcpClient *m_client;
    QLabel *m_statusLabel;
    QLineEdit *m_hostInput;
    QLineEdit *m_portInput;
    QPushButton *m_connectButton;
};

#endif // MAINWINDOW_H