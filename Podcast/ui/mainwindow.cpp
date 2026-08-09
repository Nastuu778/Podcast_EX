#include "mainwindow.h"
#include <QMessageBox>
#include <QSplitter>

MainWindow::MainWindow(const QString &username,
                       const QString &host,
                       quint16 port,
                       QWidget *parent)
    : QMainWindow(parent)
    , m_client(new TcpClient(this))
    , m_username(username)
    , m_host(host)
    , m_port(port)
{
    setupUI();

    // Создаём UDP-менеджер и занимаем порт
    m_udpManager = new UdpManager(this);
    m_udpManager->bind();

    // Создаём плеер для воспроизведения звука от других
    m_audioPlayer = new AudioPlayer(this);

    // Получаем аудио по UDP и воспроизводим
    connect(m_udpManager, &UdpManager::audioReceived, this,
            [this](const QByteArray &audio, const QString &senderName) {
                m_audioPlayer->playChunk(audio);
            });

    connect(m_client, &TcpClient::connected, this, &MainWindow::onClientConnected);
    connect(m_client, &TcpClient::disconnected, this, &MainWindow::onClientDisconnected);
    connect(m_client, &TcpClient::errorOccurred, this, &MainWindow::onClientError);
    connect(m_client, &TcpClient::textMessageReceived, this, &MainWindow::onTextMessageReceived);

    // Подключаемся к серверу
    m_client->connectToServer(m_host, m_port);
}

MainWindow::~MainWindow()
{
    m_client->disconnectFromServer();
}

void MainWindow::setupUI()
{
    setWindowTitle("Podcast Client - " + m_username);
    resize(900, 600);

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // === ЛЕВАЯ ПАНЕЛЬ - Списки участников ===
    QGroupBox *participantsGroup = new QGroupBox("Участники");
    QVBoxLayout *participantsLayout = new QVBoxLayout(participantsGroup);

    // Спикеры
    QLabel *speakersLabel = new QLabel("🎙️ Спикеры:");
    speakersLabel->setFont(QFont("Arial", 10, QFont::Bold));
    participantsLayout->addWidget(speakersLabel);

    m_speakersList = new QListWidget();
    m_speakersList->setMinimumHeight(150);
    participantsLayout->addWidget(m_speakersList);

    // Слушатели
    QLabel *listenersLabel = new QLabel(" Слушатели:");
    listenersLabel->setFont(QFont("Arial", 10, QFont::Bold));
    participantsLayout->addWidget(listenersLabel);

    m_listenersList = new QListWidget();
    m_listenersList->setMinimumHeight(200);
    participantsLayout->addWidget(m_listenersList);

    mainLayout->addWidget(participantsGroup, 1);

    // === ПРАВАЯ ПАНЕЛЬ - Чат и статус ===
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // Статус и роль
    QHBoxLayout *statusLayout = new QHBoxLayout();

    m_roleLabel = new QLabel("Роль: Слушатель");
    m_roleLabel->setFont(QFont("Arial", 10, QFont::Bold));
    statusLayout->addWidget(m_roleLabel);

    // НОВОЕ: Добавляем имя пользователя
    QLabel *userLabel = new QLabel("Пользователь: " + m_username);
    userLabel->setFont(QFont("Arial", 10, QFont::Bold));
    statusLayout->addWidget(userLabel);

    m_statusLabel = new QLabel("Подключение...");
    statusLayout->addWidget(m_statusLabel);

    rightLayout->addLayout(statusLayout);

    // Область чата
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setPlaceholderText("Здесь будут отображаться сообщения...");
    rightLayout->addWidget(m_chatDisplay);

    // Ввод сообщения (только для спикеров и слушателей - все могут писать в чат)
    QHBoxLayout *messageLayout = new QHBoxLayout();
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Введите сообщение...");
    m_messageInput->setEnabled(false);
    messageLayout->addWidget(m_messageInput);

    m_sendButton = new QPushButton("Отправить");
    m_sendButton->setEnabled(false);
    messageLayout->addWidget(m_sendButton);

    rightLayout->addLayout(messageLayout);

    // Кнопка отключения
    m_disconnectButton = new QPushButton("Отключиться");
    m_disconnectButton->setEnabled(false);
    rightLayout->addWidget(m_disconnectButton);

    mainLayout->addLayout(rightLayout, 3);

    setCentralWidget(centralWidget);

    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessageClicked);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::onSendMessageClicked);
    connect(m_disconnectButton, &QPushButton::clicked, m_client, &TcpClient::disconnectFromServer);
}

void MainWindow::onClientConnected()
{
    updateStatus("Подключено");
    m_messageInput->setEnabled(true);
    m_sendButton->setEnabled(true);
    m_disconnectButton->setEnabled(true);

    // Отправляем информацию о себе на сервер
    sendRoleToServer();
    // Сообщаем серверу наш UDP-порт для приёма аудио
    m_client->sendTextMessage(QString("/udpport:%1").arg(m_udpManager->localPort()));

    // НЕ добавляем сообщение о подключении!
    // Сервер сам отправит его после обработки /join:
}

void MainWindow::onClientDisconnected()
{
    updateStatus("Отключено");
    m_messageInput->setEnabled(false);
    m_sendButton->setEnabled(false);
    m_disconnectButton->setEnabled(false);
    appendChatMessage("Вы отключились от подкаста");
}

void MainWindow::onClientError(const QString &error)
{
    updateStatus("Ошибка: " + error);
    QMessageBox::critical(this, "Ошибка подключения", error);
}

void MainWindow::onSendMessageClicked()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty()) return;

    QString fullMessage = m_username + ": " + message;
    m_client->sendTextMessage(fullMessage);
    appendChatMessage(fullMessage);
    m_messageInput->clear();
}

void MainWindow::onTextMessageReceived(const QString &message)
{
    // Проверяем, это список участников?
    if (message.startsWith("/participants:")) {
        updateParticipantList(message);
        return;
    }

    // НОВОЕ: Проверяем, это сообщение об ошибке?
    if (message.startsWith("/error:")) {
        QString errorMessage = message.mid(7);  // Убираем "/error:"
        QMessageBox::warning(this, "Ошибка подключения", errorMessage);
        m_client->disconnectFromServer();  // Отключаемся от сервера
        close();  // Закрываем главное окно
        return;
    }

    // Игнорируем сырые команды
    if (message.startsWith("/join:")) {
        return;
    }

    // Обычное сообщение чата или системное
    appendChatMessage(message);
}

void MainWindow::updateParticipantList(const QString &data)
{
    // Формат: /participants:speakers:user1,user2;listeners:user3,user4
    // "/participants:" = 14 символов
    QString cleanData = data.mid(14);

    QStringList parts = cleanData.split(';');
    if (parts.size() != 2) return;

    // === Спикеры ===
    QString speakersPart = parts[0];
    if (speakersPart.startsWith("speakers:")) {
        QString speakersStr = speakersPart.mid(9);  // "speakers:" = 9 символов
        QStringList speakers = speakersStr.split(',');

        m_speakersList->clear();
        for (const QString &speaker : speakers) {
            if (!speaker.trimmed().isEmpty()) {
                m_speakersList->addItem(speaker.trimmed());
            }
        }
    }

    // === Слушатели ===
    QString listenersPart = parts[1];
    if (listenersPart.startsWith("listeners:")) {
        QString listenersStr = listenersPart.mid(10);  // "listeners:" = 10 символов
        QStringList listeners = listenersStr.split(',');

        m_listenersList->clear();
        for (const QString &listener : listeners) {
            if (!listener.trimmed().isEmpty()) {
                m_listenersList->addItem(listener.trimmed());
            }
        }
    }
}

void MainWindow::updateStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void MainWindow::appendChatMessage(const QString &message)
{
    m_chatDisplay->append(message);
}

void MainWindow::sendRoleToServer()
{
    QString message = QString("/join:%1:listener").arg(m_username);
    m_client->sendTextMessage(message);
}