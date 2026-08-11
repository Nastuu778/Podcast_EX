#include <QApplication>
#include <QDebug>
#include "ui/roledialog.h"
#include "ui/mainwindow.h"
#include "ui/speakerwindow.h"
#include "ui/styles.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Podcast Client");

    app.setStyleSheet(Styles::applicationStyle());

    RoleDialog roleDialog;
    if (roleDialog.exec() == QDialog::Accepted) {
        // В зависимости от роли открываем разное окно
        if (roleDialog.selectedRole() == RoleDialog::Speaker) {
            // Открываем окно спикера
            SpeakerWindow *window = new SpeakerWindow(
                roleDialog.username(),
                roleDialog.serverHost(),
                roleDialog.serverPort()
                );
            window->show();
        } else {
            // Открываем окно слушателя
            MainWindow *window = new MainWindow(
                roleDialog.username(),
                roleDialog.serverHost(),
                roleDialog.serverPort()
                );
            window->show();
        }
    } else {
        qDebug() << "Подключение отменено";
        return 0;
    }

    return app.exec();
}