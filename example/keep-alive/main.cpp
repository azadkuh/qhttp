#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"
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

#include "../include/ticktock.hxx"
#include "../include/unixcatcher.hpp"
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
///////////////////////////////////////////////////////////////////////////////

namespace client {
///////////////////////////////////////////////////////////////////////////////
class Client
{
public:
    void            start(const QString& portOrPath, int count) {
        QUrl url;
        if ( portOrPath.toUShort() > 0 ){
            url.setScheme("http");
            url.setHost("localhost");
            url.setPort(portOrPath.toUShort());

        } else {
            url = QUrl::fromLocalFile(portOrPath);
        }

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
} // namespace client

///////////////////////////////////////////////////////////////////////////////

namespace server {
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
                           if ( req->headers().keyHasValue("command", "quit") ) {
                               printf("a client sends a quit command.\nserver quits.\n");
                               QCoreApplication::quit();
                               return;
                           }

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
} // namespace server

///////////////////////////////////////////////////////////////////////////////
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});

    app.setApplicationName("keep-alive");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("a server/client application to test the performance of keep-alive connections.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("mode",
                                 "mode: server or client. default: local");

    parser.addOption(QCommandLineOption(QStringList() << "p" << "port",
                                        "tcp port number or UNIX socket path. default: 10022",
                                        "number/path", "10022"));
    parser.addOption(QCommandLineOption(QStringList() << "c" << "count",
                                        "count of request/response pairs (only in client mode). default: 100",
                                        "number", "100"));
    parser.process(app);

    QStringList posList = parser.positionalArguments();

    if ( posList.size() == 1    &&    posList.at(0) == QLatin1Literal("server") ) {
        qhttp::server::Server server;
        server.start(parser.value("port"));
        app.exec();

    } else if ( posList.size() == 1    &&    posList.at(0) == QLatin1Literal("client") ) {
        qhttp::client::Client client;
        client.start(parser.value("port"),
                     parser.value("count").toInt());
        app.exec();

    } else {
        parser.showHelp(0);
    }

    return 0;
}
