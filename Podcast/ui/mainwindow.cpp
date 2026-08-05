#include "mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_client(new TcpClient(this))
{
    setupUI();

    connect(m_client, &TcpClient::connected, this, &MainWindow::onClientConnected);
    connect(m_client, &TcpClient::disconnected, this, &MainWindow::onClientDisconnected);
    connect(m_client, &TcpClient::errorOccurred, this, &MainWindow::onClientError);
    connect(m_client, &TcpClient::textMessageReceived, this, &MainWindow::onTextMessageReceived);  // НОВОЕ
}

MainWindow::~MainWindow()
{
    m_client->disconnectFromServer();
}

void MainWindow::setupUI()
{
    setWindowTitle("Podcast Client");
    resize(500, 400);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // === Блок подключения ===
    QHBoxLayout *connectLayout = new QHBoxLayout();

    m_hostInput = new QLineEdit("127.0.0.1");
    m_hostInput->setPlaceholderText("Адрес сервера");
    connectLayout->addWidget(m_hostInput);

    m_portInput = new QLineEdit("5000");
    m_portInput->setPlaceholderText("Порт");
    m_portInput->setMaximumWidth(100);
    connectLayout->addWidget(m_portInput);

    m_connectButton = new QPushButton("Подключиться");
    connectLayout->addWidget(m_connectButton);

    mainLayout->addLayout(connectLayout);

    // === Статус ===
    m_statusLabel = new QLabel("Статус: Не подключено");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    // === Область чата ===
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setPlaceholderText("Здесь будут отображаться сообщения...");
    mainLayout->addWidget(m_chatDisplay);

    // === Ввод сообщения ===
    QHBoxLayout *messageLayout = new QHBoxLayout();

    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Введите сообщение...");
    m_messageInput->setEnabled(false);  // Пока не подключено
    messageLayout->addWidget(m_messageInput);

    m_sendButton = new QPushButton("Отправить");
    m_sendButton->setEnabled(false);  // Пока не подключено
    messageLayout->addWidget(m_sendButton);

    mainLayout->addLayout(messageLayout);

    setCentralWidget(centralWidget);

    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessageClicked);

    // Отправка по Enter
    connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendMessageClicked);
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
    updateStatus("Клиент подключён к серверу");
    m_connectButton->setText("Отключиться");
    m_messageInput->setEnabled(true);
    m_sendButton->setEnabled(true);
    appendChatMessage("Вы подключились к серверу");
}

void MainWindow::onClientDisconnected()
{
    updateStatus("Клиент отключён от сервера");
    m_connectButton->setText("Подключиться");
    m_messageInput->setEnabled(false);
    m_sendButton->setEnabled(false);
    appendChatMessage(" Соединение разорвано");
}

void MainWindow::onClientError(const QString &error)
{
    updateStatus("Ошибка: " + error);
    m_connectButton->setText("Подключиться");
    m_messageInput->setEnabled(false);
    m_sendButton->setEnabled(false);
}

void MainWindow::onSendMessageClicked()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty()) return;

    m_client->sendTextMessage(message);
    appendChatMessage("Вы: " + message);  // Показываем своё сообщение
    m_messageInput->clear();
}

void MainWindow::onTextMessageReceived(const QString &message)
{
    appendChatMessage("Сервер: " + message);
}

void MainWindow::updateStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void MainWindow::appendChatMessage(const QString &message)
{
    m_chatDisplay->append(message);
}