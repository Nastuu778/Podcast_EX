#ifndef SPEAKERWINDOW_H
#define SPEAKERWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QComboBox>
#include <QTimer>
#include <QListWidgetItem>
#include "network/udpmanager.h"
#include "audio/audiomanager.h"
#include "network/tcpclient.h"
#include "audio/audioplayer.h"

class SpeakerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SpeakerWindow(const QString &username,
                           const QString &host,
                           quint16 port,
                           QWidget *parent = nullptr);
    ~SpeakerWindow();

private slots:
    void onClientConnected();
    void onClientDisconnected();
    void onClientError(const QString &error);
    void onSendMessageClicked();
    void onTextMessageReceived(const QString &message);
    void updateParticipantList(const QString &data);
    void onMicrophoneToggle(bool checked);  // НОВОЕ

private:
    void setupUI();
    void updateStatus(const QString &status);
    void appendChatMessage(const QString &message);
    void sendRoleToServer();
    void highlightSpeaker(const QString &name);
    void clearSpeakingHighlight();

    AudioPlayer *m_audioPlayer;
    TcpClient *m_client;
    UdpManager *m_udpManager;
    QTimer *m_speakingTimer;
    QString m_username;
    QString m_host;
    quint16 m_port;

    // UI элементы
    QLabel *m_statusLabel;
    QLabel *m_roleLabel;
    QLabel *m_userLabel;
    QListWidget *m_speakersList;
    QListWidget *m_listenersList;
    QTextEdit *m_chatDisplay;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;
    QPushButton *m_disconnectButton;

    // НОВОЕ: Элементы для микрофона
    QPushButton *m_micButton;
    QLabel *m_micStatusLabel;
    QProgressBar *m_audioLevelBar;
    QComboBox *m_micComboBox;      // Выбор микрофона
    AudioManager *m_audioManager;  // Менеджер аудио
};

#endif // SPEAKERWINDOW_H