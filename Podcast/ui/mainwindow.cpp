#include "mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_client(new TcpClient(this))
    , m_clientId(QUuid::createUuid().toString(QUuid::WithoutBraces))  // Правильный синтаксис
{
    setupUI();

    connect(m_client, &TcpClient::connected, this, &MainWindow::onClientConnected);
    connect(m_client, &TcpClient::disconnected, this, &MainWindow::onClientDisconnected);
    connect(m_client, &TcpClient::errorOccurred, this, &MainWindow::onClientError);
    connect(m_client, &TcpClient::textMessageReceived, this, &MainWindow::onTextMessageReceived);

    qDebug() << "Client ID generated:" << m_clientId;  // Для отладки
}

MainWindow::~MainWindow()
{
    m_client->disconnectFromServer();
}

void MainWindow::setupUI()
{
    setWindowTitle("Podcast Client");
    resize(500, 450);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // === Блок подключения ===
    QGroupBox *connectGroup = new QGroupBox("Подключение к серверу");
    QVBoxLayout *connectLayout = new QVBoxLayout(connectGroup);

    // Поле имени пользователя
    QHBoxLayout *usernameLayout = new QHBoxLayout();
    usernameLayout->addWidget(new QLabel("Ваше имя:"));
    m_usernameInput = new QLineEdit();
    m_usernameInput->setPlaceholderText("Обязательно для заполнения");
    m_usernameInput->setMaxLength(50);
    usernameLayout->addWidget(m_usernameInput);
    connectLayout->addLayout(usernameLayout);

    // Адрес и порт
    QHBoxLayout *hostLayout = new QHBoxLayout();
    hostLayout->addWidget(new QLabel("Сервер:"));
    m_hostInput = new QLineEdit("127.0.0.1");
    m_hostInput->setPlaceholderText("Адрес сервера");
    hostLayout->addWidget(m_hostInput);

    m_portInput = new QLineEdit("5000");
    m_portInput->setPlaceholderText("Порт");
    m_portInput->setMaximumWidth(100);
    hostLayout->addWidget(m_portInput);
    connectLayout->addLayout(hostLayout);

    m_connectButton = new QPushButton("Подключиться");
    connectLayout->addWidget(m_connectButton);

    mainLayout->addWidget(connectGroup);

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
    m_messageInput->setEnabled(false);
    messageLayout->addWidget(m_messageInput);

    m_sendButton = new QPushButton("Отправить");
    m_sendButton->setEnabled(false);
    messageLayout->addWidget(m_sendButton);

    mainLayout->addLayout(messageLayout);

    setCentralWidget(centralWidget);

    connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessageClicked);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendMessageClicked);
}

bool MainWindow::validateUsername()
{
    m_username = m_usernameInput->text().trimmed();

    if (m_username.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, введите ваше имя!");
        m_usernameInput->setFocus();
        return false;
    }

    if (m_username.length() < 1) {
        QMessageBox::warning(this, "Ошибка", "Имя должно содержать минимум 1 символ!");
        m_usernameInput->setFocus();
        return false;
    }

    return true;
}

void MainWindow::onConnectButtonClicked()
{
    if (!m_client->isConnected()) {
        if (!validateUsername()) {
            return;
        }

        QString host = m_hostInput->text();
        quint16 port = m_portInput->text().toUShort();

        m_client->connectToServer(host, port);
        m_connectButton->setText("Отключиться");

        m_usernameInput->setEnabled(false);
        m_hostInput->setEnabled(false);
        m_portInput->setEnabled(false);
    } else {
        m_client->disconnectFromServer();
        m_connectButton->setText("Подключиться");

        m_usernameInput->setEnabled(true);
        m_hostInput->setEnabled(true);
        m_portInput->setEnabled(true);
    }
}

void MainWindow::onClientConnected()
{
    updateStatus("Подключено как: " + m_username);
    m_connectButton->setText("Отключиться");
    m_messageInput->setEnabled(true);
    m_sendButton->setEnabled(true);
    appendChatMessage(m_username + " подключился к серверу");
}

void MainWindow::onClientDisconnected()
{
    updateStatus("Клиент отключён от сервера");
    m_connectButton->setText("Подключиться");
    m_messageInput->setEnabled(false);
    m_sendButton->setEnabled(false);
    appendChatMessage("Соединение разорвано");

    m_usernameInput->setEnabled(true);
    m_hostInput->setEnabled(true);
    m_portInput->setEnabled(true);
}

void MainWindow::onClientError(const QString &error)
{
    updateStatus("Ошибка: " + error);
    m_connectButton->setText("Подключиться");
    m_messageInput->setEnabled(false);
    m_sendButton->setEnabled(false);

    m_usernameInput->setEnabled(true);
    m_hostInput->setEnabled(true);
    m_portInput->setEnabled(true);
}

void MainWindow::onSendMessageClicked()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty()) return;

    // Отправляем сообщение с username
    QString fullMessage = m_username + ": " + message;
    m_client->sendTextMessage(fullMessage);
    appendChatMessage(m_username + ": " + message);
    m_messageInput->clear();
}

void MainWindow::onTextMessageReceived(const QString &message)
{
    appendChatMessage(message);
}

void MainWindow::updateStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void MainWindow::appendChatMessage(const QString &message)
{
    m_chatDisplay->append(message);
}