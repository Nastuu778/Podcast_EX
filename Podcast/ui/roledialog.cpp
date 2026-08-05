#include "roledialog.h"
#include <QMessageBox>
#include <QIntValidator>

RoleDialog::RoleDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

RoleDialog::~RoleDialog()
{
}

void RoleDialog::setupUI()
{
    setWindowTitle("Подключение к подкасту");
    setMinimumSize(400, 350);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // === Поле имени пользователя ===
    QGroupBox *userGroup = new QGroupBox("Пользователь");
    QVBoxLayout *userLayout = new QVBoxLayout(userGroup);

    QHBoxLayout *usernameLayout = new QHBoxLayout();
    usernameLayout->addWidget(new QLabel("Ваше имя:"));
    m_usernameInput = new QLineEdit();
    m_usernameInput->setPlaceholderText("Введите ваше имя");
    m_usernameInput->setMaxLength(50);
    usernameLayout->addWidget(m_usernameInput);
    userLayout->addLayout(usernameLayout);
    mainLayout->addWidget(userGroup);

    // === Подключение к серверу ===
    QGroupBox *serverGroup = new QGroupBox("Подключение к серверу");
    QVBoxLayout *serverLayout = new QVBoxLayout(serverGroup);

    QHBoxLayout *hostLayout = new QHBoxLayout();
    hostLayout->addWidget(new QLabel("Сервер:"));
    m_hostInput = new QLineEdit("127.0.0.1");
    m_hostInput->setPlaceholderText("Адрес сервера");
    hostLayout->addWidget(m_hostInput);
    serverLayout->addLayout(hostLayout);

    QHBoxLayout *portLayout = new QHBoxLayout();
    portLayout->addWidget(new QLabel("Порт:"));
    m_portInput = new QLineEdit("5000");
    m_portInput->setValidator(new QIntValidator(1, 65535, this));
    portLayout->addWidget(m_portInput);
    serverLayout->addLayout(portLayout);
    mainLayout->addWidget(serverGroup);

    // === Выбор роли ===
    QGroupBox *roleGroup = new QGroupBox("Выберите роль");
    QVBoxLayout *roleLayout = new QVBoxLayout(roleGroup);

    m_speakerRadio = new QRadioButton(" Спикер (может говорить)");
    m_listenerRadio = new QRadioButton(" Слушатель (только слушает)");

    m_speakerRadio->setChecked(true);  // По умолчанию спикер

    roleLayout->addWidget(m_speakerRadio);
    roleLayout->addWidget(m_listenerRadio);
    mainLayout->addWidget(roleGroup);

    // === Кнопка подключения ===
    m_connectButton = new QPushButton("Подключиться");
    mainLayout->addWidget(m_connectButton);

    // === Метка ошибок ===
    m_errorLabel = new QLabel();
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_errorLabel);

    connect(m_connectButton, &QPushButton::clicked, this, &RoleDialog::onConnectClicked);
}

QString RoleDialog::username() const
{
    return m_usernameInput->text().trimmed();
}

QString RoleDialog::serverHost() const
{
    return m_hostInput->text();
}

quint16 RoleDialog::serverPort() const
{
    return m_portInput->text().toUShort();
}

RoleDialog::Role RoleDialog::selectedRole() const
{
    return m_speakerRadio->isChecked() ? Speaker : Listener;
}

void RoleDialog::onConnectClicked()
{
    // Проверка имени
    if (username().isEmpty()) {
        m_errorLabel->setText("Пожалуйста, введите ваше имя!");
        return;
    }

    if (username().length() < 2) {
        m_errorLabel->setText("Имя должно содержать минимум 2 символа!");
        return;
    }

    // Проверка сервера
    if (serverHost().isEmpty()) {
        m_errorLabel->setText("Пожалуйста, введите адрес сервера!");
        return;
    }

    m_errorLabel->clear();
    accept();  // Закрываем диалог с результатом QDialog::Accepted
}