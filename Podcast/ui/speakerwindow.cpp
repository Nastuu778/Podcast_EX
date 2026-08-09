#include "speakerwindow.h"
#include <QMessageBox>

SpeakerWindow::SpeakerWindow(const QString &username,
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

    // Создаём менеджер аудио
    m_audioManager = new AudioManager(this);

    // Заполняем список микрофонов
    m_micComboBox->addItems(m_audioManager->availableMicrophones());

    // Создаём UDP-менеджер и занимаем порт
    m_udpManager = new UdpManager(this);
    m_udpManager->bind();

    // Аудио-порт сервера = TCP-порт + 1
    m_udpManager->setServerAddress(m_host, static_cast<quint16>(m_port + 1));

    // Создаём плеер для воспроизведения звука от других
    m_audioPlayer = new AudioPlayer(this);

    // Получаем аудио по UDP и воспроизводим
    connect(m_udpManager, &UdpManager::audioReceived, this,
            [this](const QByteArray &audio, const QString &senderName) {
                m_audioPlayer->playChunk(audio);
                highlightSpeaker(senderName);  // Подсвечиваем говорящего
            });

    // Таймер для сброса подсветки, когда спикер замолкает
    m_speakingTimer = new QTimer(this);
    m_speakingTimer->setSingleShot(true);
    m_speakingTimer->setInterval(400);
    connect(m_speakingTimer, &QTimer::timeout, this, [this]() {
        clearSpeakingHighlight();
    });

    // Подключаем сигналы клиента
    connect(m_client, &TcpClient::connected, this, &SpeakerWindow::onClientConnected);
    connect(m_client, &TcpClient::disconnected, this, &SpeakerWindow::onClientDisconnected);
    connect(m_client, &TcpClient::errorOccurred, this, &SpeakerWindow::onClientError);
    connect(m_client, &TcpClient::textMessageReceived, this, &SpeakerWindow::onTextMessageReceived);

    // Подключаем сигналы аудио
    connect(m_audioManager, &AudioManager::audioLevelChanged, m_audioLevelBar, &QProgressBar::setValue);
    connect(m_audioManager, &AudioManager::captureError, this, [this](const QString &error) {
        m_micStatusLabel->setText("Ошибка: " + error);
        m_micStatusLabel->setStyleSheet("color: red;");
        m_micButton->setChecked(false);
    });

    // Когда микрофон захватывает звук - отправляем его на сервер
    connect(m_audioManager, &AudioManager::audioDataReady, this, [this](const QByteArray &data) {
        if (m_micButton->isChecked()) {
            m_udpManager->sendAudio(m_username, data);
            highlightSpeaker(m_username);  // Подсвечиваем СЕБЯ, когда говорим
        }
    });

    m_client->connectToServer(m_host, m_port);
}

SpeakerWindow::~SpeakerWindow()
{
    m_audioManager->stopCapture();
    m_client->disconnectFromServer();
}

void SpeakerWindow::setupUI()
{
    setWindowTitle("Podcast - Спикер: " + m_username);
    resize(900, 600);

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // === ЛЕВАЯ ПАНЕЛЬ - Участники ===
    QGroupBox *participantsGroup = new QGroupBox("Участники");
    QVBoxLayout *participantsLayout = new QVBoxLayout(participantsGroup);

    QLabel *speakersLabel = new QLabel("Спикеры:");
    speakersLabel->setFont(QFont("Arial", 10, QFont::Bold));
    participantsLayout->addWidget(speakersLabel);

    m_speakersList = new QListWidget();
    m_speakersList->setMinimumHeight(150);
    participantsLayout->addWidget(m_speakersList);

    QLabel *listenersLabel = new QLabel("Слушатели:");
    listenersLabel->setFont(QFont("Arial", 10, QFont::Bold));
    participantsLayout->addWidget(listenersLabel);

    m_listenersList = new QListWidget();
    m_listenersList->setMinimumHeight(200);
    participantsLayout->addWidget(m_listenersList);

    mainLayout->addWidget(participantsGroup, 1);

    // === ПРАВАЯ ПАНЕЛЬ ===
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // === БЛОК СПИКЕРА ===
    QGroupBox *speakerGroup = new QGroupBox("Управление микрофоном");
    QVBoxLayout *speakerLayout = new QVBoxLayout(speakerGroup);

    // Выбор микрофона
    QHBoxLayout *deviceLayout = new QHBoxLayout();
    deviceLayout->addWidget(new QLabel("Микрофон:"));
    m_micComboBox = new QComboBox();
    deviceLayout->addWidget(m_micComboBox);
    speakerLayout->addLayout(deviceLayout);

    // Кнопка микрофона
    QHBoxLayout *micLayout = new QHBoxLayout();
    m_micButton = new QPushButton("Включить микрофон");
    m_micButton->setCheckable(true);
    m_micButton->setEnabled(false);  // Пока нет подключения
    m_micButton->setStyleSheet(
        "QPushButton { background-color: #e0e0e0; padding: 10px; font-weight: bold; }"
        "QPushButton:checked { background-color: #4CAF50; color: white; }"
        );
    micLayout->addWidget(m_micButton);
    speakerLayout->addLayout(micLayout);

    m_micStatusLabel = new QLabel("Микрофон выключен");
    m_micStatusLabel->setAlignment(Qt::AlignCenter);
    speakerLayout->addWidget(m_micStatusLabel);

    // Индикатор уровня звука
    m_audioLevelBar = new QProgressBar();
    m_audioLevelBar->setRange(0, 100);
    m_audioLevelBar->setValue(0);
    m_audioLevelBar->setTextVisible(false);
    speakerLayout->addWidget(m_audioLevelBar);

    rightLayout->addWidget(speakerGroup);

    // === Информация о пользователе ===
    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_roleLabel = new QLabel("Роль: Спикер");
    m_roleLabel->setFont(QFont("Arial", 10, QFont::Bold));
    statusLayout->addWidget(m_roleLabel);

    m_userLabel = new QLabel("Пользователь: " + m_username);
    m_userLabel->setFont(QFont("Arial", 10, QFont::Bold));
    statusLayout->addWidget(m_userLabel);

    m_statusLabel = new QLabel("Подключение...");
    statusLayout->addWidget(m_statusLabel);

    rightLayout->addLayout(statusLayout);

    // === Чат ===
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    rightLayout->addWidget(m_chatDisplay);

    QHBoxLayout *messageLayout = new QHBoxLayout();
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Введите сообщение...");
    m_messageInput->setEnabled(false);
    messageLayout->addWidget(m_messageInput);

    m_sendButton = new QPushButton("Отправить");
    m_sendButton->setEnabled(false);
    messageLayout->addWidget(m_sendButton);

    rightLayout->addLayout(messageLayout);

    m_disconnectButton = new QPushButton("Отключиться");
    m_disconnectButton->setEnabled(false);
    rightLayout->addWidget(m_disconnectButton);

    mainLayout->addLayout(rightLayout, 3);

    setCentralWidget(centralWidget);

    connect(m_sendButton, &QPushButton::clicked, this, &SpeakerWindow::onSendMessageClicked);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &SpeakerWindow::onSendMessageClicked);
    connect(m_disconnectButton, &QPushButton::clicked, m_client, &TcpClient::disconnectFromServer);
    connect(m_micButton, &QPushButton::toggled, this, &SpeakerWindow::onMicrophoneToggle);
}

void SpeakerWindow::onMicrophoneToggle(bool checked)
{
    if (checked) {
        // Запускаем захват с выбранного микрофона
        QString deviceName = m_micComboBox->currentText();
        if (!m_audioManager->startCapture(deviceName)) {
            m_micButton->setChecked(false);
            return;
        }

        m_micComboBox->setEnabled(false);  // Блокируем смену микрофона во время записи
        m_micButton->setText("Выключить микрофон");
        m_micStatusLabel->setText("Микрофон включён - вы говорите!");
        m_micStatusLabel->setStyleSheet("color: green; font-weight: bold;");
        appendChatMessage("[Система] Вы включили микрофон");

        // TODO: Здесь будет отправка аудио по UDP
    } else {
        m_audioManager->stopCapture();

        m_micComboBox->setEnabled(true);
        m_micButton->setText("Включить микрофон");
        m_micStatusLabel->setText("Микрофон выключен");
        m_micStatusLabel->setStyleSheet("color: gray;");
        appendChatMessage("[Система] Вы выключили микрофон");

        // TODO: Здесь будет остановка отправки аудио
    }
}

void SpeakerWindow::onClientConnected()
{
    updateStatus("Подключено");
    m_messageInput->setEnabled(true);
    m_sendButton->setEnabled(true);
    m_disconnectButton->setEnabled(true);
    m_micButton->setEnabled(true);  // Активируем кнопку микрофона

    sendRoleToServer();
    // Сообщаем серверу наш UDP-порт для приёма аудио
    m_client->sendTextMessage(QString("/udpport:%1").arg(m_udpManager->localPort()));
}

void SpeakerWindow::onClientDisconnected()
{
    updateStatus("Отключено");
    m_messageInput->setEnabled(false);
    m_sendButton->setEnabled(false);
    m_disconnectButton->setEnabled(false);
    m_micButton->setEnabled(false);

    if (m_micButton->isChecked()) {
        m_micButton->setChecked(false);
    }

    appendChatMessage("Вы отключились от подкаста");
}

void SpeakerWindow::onClientError(const QString &error)
{
    updateStatus("Ошибка: " + error);
    QMessageBox::critical(this, "Ошибка подключения", error);
}

void SpeakerWindow::onSendMessageClicked()
{
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty()) return;

    QString fullMessage = m_username + ": " + message;
    m_client->sendTextMessage(fullMessage);
    appendChatMessage(fullMessage);
    m_messageInput->clear();
}

void SpeakerWindow::onTextMessageReceived(const QString &message)
{
    if (message.startsWith("/participants:")) {
        updateParticipantList(message);
        return;
    }

    if (message.startsWith("/error:")) {
        QString errorMessage = message.mid(7);
        QMessageBox::warning(this, "Ошибка подключения", errorMessage);
        m_client->disconnectFromServer();
        close();
        return;
    }

    if (message.startsWith("/join:")) {
        return;
    }

    appendChatMessage(message);
}

void SpeakerWindow::updateParticipantList(const QString &data)
{
    QString cleanData = data.mid(14);

    QStringList parts = cleanData.split(';');
    if (parts.size() != 2) return;

    QString speakersPart = parts[0];
    if (speakersPart.startsWith("speakers:")) {
        QString speakersStr = speakersPart.mid(9);
        QStringList speakers = speakersStr.split(',');
        m_speakersList->clear();
        for (const QString &speaker : speakers) {
            if (!speaker.trimmed().isEmpty()) {
                m_speakersList->addItem(speaker.trimmed());
            }
        }
    }

    QString listenersPart = parts[1];
    if (listenersPart.startsWith("listeners:")) {
        QString listenersStr = listenersPart.mid(10);
        QStringList listeners = listenersStr.split(',');
        m_listenersList->clear();
        for (const QString &listener : listeners) {
            if (!listener.trimmed().isEmpty()) {
                m_listenersList->addItem(listener.trimmed());
            }
        }
    }
}

void SpeakerWindow::updateStatus(const QString &status)
{
    m_statusLabel->setText(status);
}

void SpeakerWindow::appendChatMessage(const QString &message)
{
    m_chatDisplay->append(message);
}

void SpeakerWindow::sendRoleToServer()
{
    QString message = QString("/join:%1:speaker").arg(m_username);
    m_client->sendTextMessage(message);
}

void SpeakerWindow::highlightSpeaker(const QString &name)
{
    for (int i = 0; i < m_speakersList->count(); ++i) {
        QListWidgetItem *item = m_speakersList->item(i);

        if (item->text() == name && m_speakersList->itemWidget(item) == nullptr) {
            item->setData(Qt::UserRole, name);  // Запоминаем имя
            item->setText(QString());           // Скрываем текст элемента

            QLabel *label = new QLabel(name);
            label->setStyleSheet(
                "border: 2px solid #4CAF50;"
                "border-radius: 4px;"
                "padding: 4px;"
                "color: white;"
                );
            m_speakersList->setItemWidget(item, label);
        }
    }
    m_speakingTimer->start();
}

void SpeakerWindow::clearSpeakingHighlight()
{
    for (int i = 0; i < m_speakersList->count(); ++i) {
        QListWidgetItem *item = m_speakersList->item(i);
        if (m_speakersList->itemWidget(item) != nullptr) {
            QString savedName = item->data(Qt::UserRole).toString();
            m_speakersList->removeItemWidget(item);
            item->setText(savedName);  // Возвращаем имя
        }
    }
}