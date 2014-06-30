#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QVector>
#include <QHash>


#include "httpclient.hpp"
#include "include/ticktock.hxx"

///////////////////////////////////////////////////////////////////////////////
class Application : public QCoreApplication
{
public:
    explicit Application(int &argc, char** argv) : QCoreApplication(argc, argv) {
        setApplicationName("httpclient-raw");
        setApplicationVersion("1.0.0");

        QCommandLineParser parser;
        parser.setApplicationDescription("attacks an http server by sending JSon requests and parses the responses.\n" \
                                         "by Amir Zamani.");
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addPositionalArgument("server",
                                     "optional server address as address[:port]. default: localhost:8080");

        parser.addOption(QCommandLineOption(QStringList() << "p" << "port",
                                            "server's port number. default: 8080",
                                            "portNumber", "8080"));

        parser.addOption(QCommandLineOption(QStringList() << "s" << "server",
                                            "server's address (url or ip). default: localhost",
                                            "ip", "127.0.0.1"));

        parser.addOption(QCommandLineOption(QStringList() << "c" << "count",
                                            "number of sockets to be connected to the server. default: 10",
                                            "number", "10"));

        parser.addOption(QCommandLineOption(QStringList() << "r" << "requests",
                                            "number of requests for each socket. default: 100",
                                            "number", "100"));

        parser.addOption(QCommandLineOption(QStringList() << "i" << "interval",
                                            "time interval between each request in [mSec]. default: 1000",
                                            "number", "1000"));

        parser.addOption(QCommandLineOption(QStringList() << "k" << "keep",
                                            "keep a connection alive or disconnect after each response. default: yes",
                                            "yes/no", "yes"));

        parser.process(*this);
        QStringList args = parser.positionalArguments();

        iport = 0;
        if ( args.size() >= 1 ) {
            QString server = args.at(0);
            QStringList items = server.split(":", QString::SkipEmptyParts);
            if ( items.size() >= 1 )
                iaddress = items.at(0);
            if ( items.size() >= 2 )
                iport    = items.at(1).toUShort();
        }

        if ( iport == 0 )
            iport     = parser.value("port").toUShort();
        if ( iaddress.isEmpty() )
            iaddress  = parser.value("server");
        iclientCount    = parser.value("count").toUInt();
        irequestCount   = parser.value("requests").toUInt();
        itimeOut        = parser.value("interval").toUInt();
        ikeepAlive      = parser.value("keep").toLower() == "yes";

        printf("\nconnecting to %s:%d --count %lu --requests %lu --interval %u --keep %s\n\n",
               iaddress.toUtf8().constData(), iport,
               iclientCount, irequestCount, itimeOut,
               (ikeepAlive) ? "yes": "no");
    }

    ~Application() {
    }

    void     startClients() {
        ifinishedPackets  = 0;
        ifinishedCount    = 0;

        for ( size_t i = 0;    i < iclientCount;    i++ ) {
            HttpClient* client = new HttpClient(i+1, this);
            iclients.insert(i+1, client);
            client->setAddress(iaddress);
            client->setPort(iport);
            client->setRequestCount(irequestCount);
            client->setSleepTime(itimeOut);
            client->setKeepAlive(ikeepAlive);

            QObject::connect(client, &HttpClient::finished,
                             [this](int clientId, size_t packets) {
                yetAnotherFinished(clientId, packets);
            });
        }

        itick.tick();
        for ( size_t i = 0;    i < iclientCount;    i++ ) {
            HttpClient* cli = iclients.value(i+1);
            QMetaObject::invokeMethod(cli, "start");
        }
    }

protected:
    void     yetAnotherFinished(int clientId, size_t packets) {
        ifinishedPackets  += packets;
        ifinishedCount++;

        HttpClient* client = iclients.value(clientId);
        if ( client )
            client->deleteLater();
        iclients.remove(clientId);
        printf("client #%d finished.\n", clientId);
        fflush(stdout);

        if ( ifinishedCount == iclientCount ) {
            uint32_t diff = itick.tock();
            printf("\n%lu JSON packets have been transfered by %lu sockets in %u [mSec].\n",
                   ifinishedPackets, iclientCount,
                   diff);
            quit();
        }
    }

protected:
    QString         iaddress;
    quint16         iport;
    size_t          iclientCount;
    size_t          irequestCount;
    quint32         itimeOut;
    bool            ikeepAlive;


protected:
    typedef QHash<int, HttpClient*>   TClients;

    am::TickTock    itick;
    size_t          ifinishedCount;
    size_t          ifinishedPackets;

    TClients        iclients;
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    Application app(argc, argv);
    app.startClients();


    app.exec();
    puts("\n\nend of http client.\n\n");
    return 0;
}


