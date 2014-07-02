#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "httpserver.hpp"

///////////////////////////////////////////////////////////////////////////////

class Application : public QCoreApplication
{
public:
    explicit    Application(int& argc, char** argv)
        : QCoreApplication(argc, argv), iserver(this) {
        setApplicationName("qtjsonserver");
        setApplicationVersion("1.0.0");

        QCommandLineParser parser;
        parser.setApplicationDescription("a simple http server for responding incomming JSon.\n" \
                                         "by Amir Zamani.");
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addPositionalArgument("port",
                                     "listening port. default: 5533");

       parser.addOption(QCommandLineOption(QStringList() << "t" << "timeout",
                                            "maximum timeout for an open connection. default: 5000",
                                            "number", "5000"));

        parser.process(*this);

        iport                  = 5533;
        itimeOut               = parser.value("timeout").toUInt();
        iserver.setTimeOut(itimeOut);

        QStringList args = parser.positionalArguments();
        if ( args.size() >= 1 ) {
            iport = args.at(0).toUShort();
        }

        printf("listening @ %d --timeout %u\n\n",
               iport, itimeOut);

        QObject::connect(&iserver,      &am::HttpServer::closed,
                         this,          &QCoreApplication::quit,
                         Qt::QueuedConnection
                         );
    }

    bool        start() {

        if ( !iserver.listen(QHostAddress::Any, iport) ) {
            printf("can not listen on port #%d.\n", iport);
            return false;
        }

        return true;
    }

protected:
    am::HttpServer  iserver;

    quint16         iport;
    size_t          imaxPendingConnections;
    quint32         itimeOut;
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    Application app(argc, argv);
    if ( app.start() )
        app.exec();

    puts("\n\nend of http server.\n\n");
    return 0;
}


