#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include <QtCore/QCoreApplication>
#include <QDateTime>
#include <QLocale>

#include "../include/unixcatcher.hpp"

using namespace qhttp::server;
///////////////////////////////////////////////////////////////////////////////

/** connection class for gathering incomming chunks of data from HTTP client. */
class ClientHandler : public QObject
{
public:
    explicit        ClientHandler(quint64 id, QHttpRequest* req, QHttpResponse* res)
        : QObject(req), iconnectionId(id) {
        // append chunks of data into uniform body.
        req->onData([this](const QByteArray& chunk) {
            ibody.append(chunk);
        });

        // when all the incoming data are gathered, send some response to client.
        req->onEnd([this, req, res]() {

            printf("connection (#%llu ): request from %s:%d\nurl: %s\n",
                   iconnectionId,
                   req->remoteAddress().toUtf8().constData(),
                   req->remotePort(),
                   qPrintable(req->url().toString())
                   );

            if ( req->method() == qhttp::EHTTP_POST )
                printf("    body: \"%s\"\n", ibody.constData());

            QString body = QString("Hello World\n    packet count = %1\n    time = %2\n")
                           .arg(iconnectionId)
                           .arg(QLocale::c().toString(
                                    QDateTime::currentDateTime(),
                                    "yyyy-MM-dd hh:mm:ss")
                                );

            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeaderValue("content-length", body.size());
            res->end(body.toUtf8());

            if ( req->headers().keyHasValue("command", "quit") ) {
                printf("a quit has been requested!\n");
                QCoreApplication::instance()->quit();
            }
        });
    }

    virtual        ~ClientHandler() {
        puts("~ClientHandler(): I've being called automatically!");
    }

protected:
    quint64         iconnectionId;
    QByteArray      ibody;
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});

    // dumb (trivial) connection counter
    quint64 iconnectionCounter = 0;

    QString portOrUnixSocket("10022"); // default: TCP port 10022
    if ( argc > 1 )
        portOrUnixSocket = argv[1];

    QHttpServer server(&app);
    server.listen(portOrUnixSocket, [&](QHttpRequest* req, QHttpResponse* res){
        new ClientHandler(iconnectionCounter++, req, res);
        // this ClientHandler object will be deleted automatically when:
        // socket disconnects (automatically after data has been sent to the client)
        //     -> QHttpConnection destroys
        //         -> QHttpRequest destroys -> ClientHandler destroys
        //         -> QHttpResponse destroys
        // all by parent-child model of QObject.
    });

    if ( !server.isListening() ) {
        fprintf(stderr, "can not listen on %s!\n", qPrintable(portOrUnixSocket));
        return -1;
    }

    app.exec();
}
