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

#include <signal.h>
#include <unistd.h>

#include "../include/ticktock.hxx"
///////////////////////////////////////////////////////////////////////////////
using namespace qhttp::client;
///////////////////////////////////////////////////////////////////////////////

class Client
{
public:
    void            start(const QUrl& url, int count) {
        iurl   = url;
        icount = count;

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
        //qDebug("stan #%d is received.\n", istan);

        if ( istan >= icount )
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
void catchUnixSignals(const std::vector<int>& quitSignals,
                      const std::vector<int>& ignoreSignals = std::vector<int>()) {

    auto handler = [](int sig) ->void {
        printf("\nquit the application (user request signal = %d).\n", sig);
        QCoreApplication::quit();
    };

    for ( int sig : ignoreSignals )
        signal(sig, SIG_IGN);

    for ( int sig : quitSignals )
        signal(sig, handler);
}

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});

    app.setApplicationName("client-keep");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("a simple client to test performance of keep-alive connections.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("backend",
                                 "backend mode, tcp or local. default: local");

    parser.addOption(QCommandLineOption(QStringList() << "p" << "port",
                                        "tcp port number or UNIX socket path of local server, default: 10022",
                                        "number/path", "10022"));
    parser.addOption(QCommandLineOption(QStringList() << "c" << "count",
                                        "count of request/response pairs, default: 100",
                                        "number", "100"));
    parser.process(app);

    QStringList posList = parser.positionalArguments();

    QUrl url;
    if ( posList.size() >= 1    &&    posList.at(0).toLower() == "tcp" ){
        url.setScheme("http");
        url.setHost("localhost");
        url.setPort(parser.value("port").toInt());

    } else {
        url = QUrl::fromLocalFile(parser.value("port"));
    }

    Client  client;
    client.start(url, parser.value("count").toUInt());

    return app.exec();
}
