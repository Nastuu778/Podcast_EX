#include <QApplication>
#include <QDebug>
#include "ui/roledialog.h"
#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Podcast Client");

    // Показываем диалог выбора роли
    RoleDialog roleDialog;
    if (roleDialog.exec() == QDialog::Accepted) {
        // Создаём главное окно с чатом
        MainWindow *window = new MainWindow(
            roleDialog.username(),
            roleDialog.serverHost(),
            roleDialog.serverPort(),
            roleDialog.selectedRole()
            );
        window->show();
    } else {
        qDebug() << "Подключение отменено";
        return 0;
    }

    return app.exec();
}