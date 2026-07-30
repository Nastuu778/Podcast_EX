#include <QApplication>
#include "network/tcpserver.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TcpServer server;
    if (server.start(5000)) {
        qDebug() << "Podcast server is running on port 5000";
    }

    return app.exec();
}