#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

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
using namespace qhttp::server;
///////////////////////////////////////////////////////////////////////////////

class Server : public QHttpServer
{
public:
    using QHttpServer::QHttpServer;

public:
    void            start(const QString pathOrPortNumber) {
        connect(this,  &QHttpServer::newConnection, [this](QHttpConnection*){
            printf("a new connection has been come!\n");
        });

        bool isListening = listen(pathOrPortNumber,
                                  [this](QHttpRequest* req, QHttpResponse* res){
                           req->collectData(512);
                           req->onEnd([this, req, res](){
                                onData(req, res);
                           });
        });

        if ( !isListening ) {
            qDebug("can not listen on %s!\n", qPrintable(pathOrPortNumber));
            QCoreApplication::quit();
        }
    }

    void            onData(QHttpRequest* req, QHttpResponse* res) {
        QVariantMap root = QJsonDocument::fromJson(req->collectedData())
                           .toVariant().toMap();
        if ( root.isEmpty()  || root.value("name").toString() != QLatin1Literal("add") ) {
            const static char KMessage[] = "Invalid JSon format!";
            res->addHeader("connection", "close");
            res->addHeaderValue("content-length", strlen(KMessage));
            res->setStatusCode(qhttp::ESTATUS_BAD_REQUEST);
            res->end(KMessage);
            return;
        }

        QVariantList args = root.value("arguments").toList();
        int total = 0;
        foreach (const QVariant& v, args) {
            total += v.toInt();
        }
        root["arguments"] = total;

        QByteArray body = QJsonDocument::fromVariant(root).toJson();
        res->addHeader("connection", "keep-alive");
        res->addHeaderValue("content-length", body.length());
        res->setStatusCode(qhttp::ESTATUS_OK);
        res->end(body);
    }

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

    app.setApplicationName("server-keep");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("a simple server to test performance of keep-alive connections.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("port",
                                 "tcp port number or UNIX socket path to listen, default: 10022");

    parser.process(app);


    Server  server(&app);

    QStringList posList = parser.positionalArguments();

    if ( posList.size() == 0 ) {
        server.start("10022");
    } else {
        server.start(posList.at(0));
    }

    return app.exec();
}
