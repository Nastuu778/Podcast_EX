#include "mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_client(new TcpClient(this))
{
    setupUI();

    // Подключаем сигналы клиента
    connect(m_client, &TcpClient::connected, this, &MainWindow::onClientConnected);
    connect(m_client, &TcpClient::disconnected, this, &MainWindow::onClientDisconnected);
    connect(m_client, &TcpClient::errorOccurred, this, &MainWindow::onClientError);
}

MainWindow::~MainWindow()
{
    m_client->disconnectFromServer();
}

void MainWindow::setupUI()
{
    setWindowTitle("Podcast Client");
    resize(400, 200);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Поле для адреса сервера
    m_hostInput = new QLineEdit("127.0.0.1");
    m_hostInput->setPlaceholderText("Адрес сервера");
    layout->addWidget(m_hostInput);

    // Поле для порта
    m_portInput = new QLineEdit("5000");
    m_portInput->setPlaceholderText("Порт");
    layout->addWidget(m_portInput);

    // Кнопка подключения
    m_connectButton = new QPushButton("Подключиться");
    layout->addWidget(m_connectButton);

    // Метка статуса
    m_statusLabel = new QLabel("Статус: Не подключено");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_statusLabel);

    setCentralWidget(centralWidget);

    // Подключаем кнопку
    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
}

void MainWindow::onConnectButtonClicked()
{
    QString host = m_hostInput->text();
    quint16 port = m_portInput->text().toUShort();

    if (m_client->isConnected()) {
        m_client->disconnectFromServer();
        m_connectButton->setText("Подключиться");
    } else {
        m_client->connectToServer(host, port);
        m_connectButton->setText("Отключиться");
    }
}

void MainWindow::onClientConnected()
{
    updateStatus("✅ Клиент подключён к серверу");
    m_connectButton->setText("Отключиться");
}

void MainWindow::onClientDisconnected()
{
    updateStatus("❌ Клиент отключён от сервера");
    m_connectButton->setText("Подключиться");
}

void MainWindow::onClientError(const QString &error)
{
    updateStatus("⚠️ Ошибка: " + error);
    m_connectButton->setText("Подключиться");
}

void MainWindow::updateStatus(const QString &status)
{
    m_statusLabel->setText(status);
}