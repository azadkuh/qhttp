#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

#include <QLocalServer>

#include "../include/ticktock.hxx"
///////////////////////////////////////////////////////////////////////////////
static const char KSocketPath[] = "/tmp/client-ka.socket";
using namespace qhttp::client;

///////////////////////////////////////////////////////////////////////////////

class Client
{
public:
    void            start(int count, quint16 ) {
        icount = count;
        iurl = QUrl("http://localhost:10022/");

        itick.tick();
        send();
    }

    void            send() {
        iclient.request(qhttp::EHTTP_POST,
                        iurl,
                        [this](QHttpRequest* req){
                            onRequest(req);
                        },
                        [this](QHttpResponse* res) {
                            res->collectData(512);

                            res->onEnd([this, res](){
                                QJsonDocument doc = QJsonDocument::fromJson(res->collectedData());
                                onData(doc.toJson());
                            });
                        });

    }

    void            onRequest(QHttpRequest* req) {
        QVariantMap cmd;
        cmd["name"]      = "add";
        cmd["stan"]      = ++istan;
        cmd["arguments"] = QVariantList{QVariant(14), QVariant(12), QVariant(-10)};
        QByteArray body  = QJsonDocument::fromVariant(cmd).toJson();

        req->addHeader("connection", "keep-alive");
        req->addHeaderValue("content-length", body.length());
        req->end(body);
    }

    void            onData(const QByteArray& data) {
        QVariantMap root = QJsonDocument::fromJson(data).toVariant().toMap();
        if ( root.isEmpty() ) {
            qDebug("the result is an invalid json\n");
            finalize();
            return;
        }

        if ( root.value("stan").toInt() != istan ) {
            qDebug("invalid stan number, %d != %d\n", istan, root.value("stan").toInt());
        }
        qDebug("stan #%d is received.\n", istan);

        if ( istan > icount )
            finalize();
        else
            send();
    }

    void            finalize() {
        qDebug("totally %d request/response pairs have been transmitted in %lu [mSec].\n",
               istan, itick.tock()
               );

        QCoreApplication::quit();
    }


public:
    QUrl            iurl;
    int             icount = 0;
    int             istan = 0;

    QHttpClient     iclient;

    am::TickTock    itick;
};


///////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);
    app.setApplicationName("client-keep");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("a simple client to test performance of keep-alive connections.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("backend",
                                 "backend mode, tcp or local. default: local");

    parser.addOption(QCommandLineOption(QStringList() << "p" << "port",
                                        "tcp port of local server (only in tcp mode), default: 10022",
                                        "number", "10022"));
    parser.addOption(QCommandLineOption(QStringList() << "c" << "count",
                                        "count of request/response pairs, default: 100",
                                        "number", "100"));
    parser.process(app);

    qhttp::TBackend backend = qhttp::ELocalSocket;

    QStringList posList = parser.positionalArguments();
    if ( posList.size() >= 1    &&    posList.at(0).toLower() == "tcp" )
        backend = qhttp::ETcpSocket;

    Client  client;
    client.start(parser.value("count").toUInt(),
                 parser.value("port").toUShort()
                 );

    return app.exec();
}
