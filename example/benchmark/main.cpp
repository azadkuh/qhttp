#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QLocalServer>

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

        parser.addOption(QCommandLineOption(QStringList() << "b" << "backend",
                                            "backend type of http connection. cant be tcp or local (local socket). default: tcp",
                                            "type", "tcp"));
        parser.addOption(QCommandLineOption(QStringList() << "t" << "timeout",
                                            "maximum timeout for an open connection. default: 5000",
                                            "number", "5000"));
        parser.addOption(QCommandLineOption(QStringList() << "p" << "port",
                                            "server's tcp port number. default: 8080",
                                            "portNumber", "8080"));
        parser.addOption(QCommandLineOption(QStringList() << "a" << "address",
                                            "server's address (url or ip[:port]), only in client mode. default: localhost",
                                            "ip", "127.0.0.1"));
        parser.addOption(QCommandLineOption(QStringList() << "c" << "count",
                                            "number of sockets to be connected to the server, only in client mode. default: 10",
                                            "number", "10"));

        parser.process(*this);

        // findout working mode
        QStringList posList = parser.positionalArguments();
        if ( posList.size() >= 1    &&    posList.at(0) == "client" )
            imode = EClient;

        itimeOut         = parser.value("timeout").toUInt();
        iport            = parser.value("port").toUShort();
        QString address  = parser.value("address");
        iconnectionCount = parser.value("count").toUInt(); // only used in client mode

        if ( parser.value("backend") == "local" ) {
            ibackend  = qhttp::ELocalSocket;
            iaddress  = "qhttp.benchmark"; // default value
            iport     = 0;

            if ( imode == EClient )
                printf("Client mode, attacking on local socket %s --count %lu --timeout %u\n\n",
                       iaddress.toUtf8().constData(),
                       iconnectionCount, itimeOut);
            else
                printf("Server mode, local socket listening @ %s --timeout %u\n\n",
                       qPrintable(iaddress), itimeOut);


        } else {
            ibackend  = qhttp::ETcpSocket;

            if ( imode == EClient ) {
                QStringList items = address.split(":", QString::SkipEmptyParts);
                if ( items.size() >= 1 )
                    iaddress = items.at(0);
                if ( items.size() >= 2 )
                    iport    = items.at(1).toUShort();

                printf("Client mode, attacking on tcp socket %s:%d --count %lu --timeout %u\n\n",
                       iaddress.toUtf8().constData(), iport,
                       iconnectionCount, itimeOut);

            } else
                printf("Server mode, tcp listening @ %d --timeout %u\n\n",
                       iport, itimeOut);
        }

    }

    virtual    ~Application() {
        if ( ibackend == qhttp::ELocalSocket )
            QLocalServer::removeServer(iaddress);
    }

    bool        initialize() {
        if ( imode == EServer ) {
            iserver = new Server(this);
            iserver->setTimeOut(itimeOut);

            return ( ibackend == qhttp::ETcpSocket ) ?
                        iserver->listen(iport) :
                        iserver->listen(iaddress);

        } else {
            iclients = new Clients(this);
            return iclients->setup(ibackend,
                                   iaddress, iport,
                                   iconnectionCount, itimeOut);
        }

        return false;
    }

protected:
    TMode           imode        = EServer;

    quint32         itimeOut;
    size_t          iconnectionCount;
    quint16         iport;
    QString         iaddress;
    qhttp::TBackend ibackend     = qhttp::ETcpSocket;

    Server*         iserver      = nullptr;
    Clients*        iclients     = nullptr;
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    Application app(argc, argv);

    if ( app.initialize() )
        app.exec();

    puts("\n\nend of QHttp benckmarking.\n\n");
    return 0;
}


