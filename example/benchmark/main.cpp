#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "server.hpp"
#include "clients.hpp"
///////////////////////////////////////////////////////////////////////////////

class Application : public QCoreApplication
{
    enum        TMode {
        EServer,
        EClient
    };

public:
    explicit    Application(int& argc, char** argv) : QCoreApplication(argc, argv) {
        setApplicationName("benchmark");
        setApplicationVersion("1.0.0");

        QCommandLineParser parser;
        parser.setApplicationDescription("a simple benchmarking tool for QHttp library.\n" \
                                         "by Amir Zamani.");
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addPositionalArgument("mode",
                                     "working mode, client or server. default: server");

        parser.addOption(QCommandLineOption(QStringList() << "t" << "timeout",
                                            "maximum timeout for an open connection. default: 5000",
                                            "number", "5000"));
        parser.addOption(QCommandLineOption(QStringList() << "p" << "port",
                                            "server's port number. default: 8080",
                                            "portNumber", "8080"));
        parser.addOption(QCommandLineOption(QStringList() << "a" << "address",
                                            "server's address (url or ip[:port]), only in client mode. default: localhost",
                                            "ip", "127.0.0.1"));
        parser.addOption(QCommandLineOption(QStringList() << "c" << "count",
                                            "number of sockets to be connected to the server, only in client mode. default: 10",
                                            "number", "10"));

        parser.process(*this);

        imode = EServer;    // default mode
        QStringList posList = parser.positionalArguments();
        if ( posList.size() >= 1    &&    posList.at(0) == "client" )
            imode = EClient;


        iport                  = parser.value("port").toUShort();
        itimeOut               = parser.value("timeout").toUInt();

        if ( imode == EClient ) {
            iconnectionCount = parser.value("count").toUInt();
            QString address  = parser.value("address");
            QStringList items = address.split(":", QString::SkipEmptyParts);
            if ( items.size() >= 1 )
                iaddress = items.at(0);
            if ( items.size() >= 2 )
                iport    = items.at(1).toUShort();
        }

        if ( imode == EServer ) {
            printf("Server mode, listening @ %d --timeout %u\n\n",
                   iport, itimeOut);
        } else {
            printf("Client mode, attacking  %s:%d --count %lu --timeout %u\n\n",
                   iaddress.toUtf8().constData(), iport,
                   iconnectionCount, itimeOut);
        }
    }

    bool        initialize() {
        if ( imode == EServer ) {
            iserver = new Server(this);
            iserver->setTimeOut(itimeOut);

            if ( iserver->listen(iport) )
                return true;

        } else {
            iclients = new Clients(this);
            if ( iclients->setup(iaddress, iport, iconnectionCount, itimeOut) )
                return true;
        }

        return false;
    }

protected:
    TMode       imode;

    quint16     iport;
    quint32     itimeOut;
    size_t      iconnectionCount;
    QString     iaddress;

    Server*     iserver  = nullptr;
    Clients*    iclients = nullptr;
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    Application app(argc, argv);

    if ( app.initialize() )
        app.exec();

    puts("\n\nend of QHttp benckmarking.\n\n");
    return 0;
}


