#include <QCoreApplication>
#include "network/tcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TcpServer server;
    if (server.start(5000)) {
        qDebug() << "🎙️  Podcast server started on port 5000";
        qDebug() << "📡 Waiting for clients...";
    } else {
        qCritical() << "❌ Failed to start server!";
        return 1;
    }

    return app.exec();
}