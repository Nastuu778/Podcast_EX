#ifndef ROLEDIALOG_H
#define ROLEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>

class RoleDialog : public QDialog
{
    Q_OBJECT

public:
    enum Role {
        Speaker,
        Listener
    };

    explicit RoleDialog(QWidget *parent = nullptr);
    ~RoleDialog();

    QString username() const;
    QString serverHost() const;
    quint16 serverPort() const;
    Role selectedRole() const;

private slots:
    void onConnectClicked();

private:
    void setupUI();

    QLineEdit *m_usernameInput;
    QLineEdit *m_hostInput;
    QLineEdit *m_portInput;
    QRadioButton *m_speakerRadio;
    QRadioButton *m_listenerRadio;
    QPushButton *m_connectButton;
    QLabel *m_errorLabel;

    QButtonGroup *m_roleGroup;
};

#endif // ROLEDIALOG_H