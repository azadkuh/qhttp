#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include <QtCore/QCoreApplication>
#include <QDateTime>
#include <QLocale>

#include "../include/unixcatcher.hpp"

namespace {
///////////////////////////////////////////////////////////////////////////////
using namespace qhttp::server;

/** connection class for gathering incoming chunks of data from HTTP client.
 * @warning please note that the incoming request instance is the parent of
 * this QObject instance. Thus this class will be deleted automatically.
 * */
class ClientHandler : public QObject
{
public:
    explicit ClientHandler(quint64 id, QHttpRequest* req, QHttpResponse* res)
        : QObject(req /* as parent*/), iconnectionId(id) {

        qDebug("connection #%llu ..." , id);

        // automatically collect http body(data) upto 1KB
        req->collectData(1024);

        // when all the incoming data are gathered, send some response to client.
        req->onEnd([this, req, res]() {

            qDebug("  connection (#%llu): request from %s:%d\n  method: %s url: %s",
                   iconnectionId,
                   req->remoteAddress().toUtf8().constData(),
                   req->remotePort(),
                   qhttp::Stringify::toString(req->method()),
                   qPrintable(req->url().toString())
                   );

            if ( req->collectedData().size() > 0 )
                qDebug("  body (#%llu): %s",
                        iconnectionId,
                        req->collectedData().constData()
                        );

            QString message =
                QString("Hello World\n  packet count = %1\n  time = %2\n")
                .arg(iconnectionId)
                .arg(QLocale::c().toString(QDateTime::currentDateTime(),
                                           "yyyy-MM-dd hh:mm:ss")
                );

            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeaderValue("content-length", message.size());
            res->end(message.toUtf8());

            if ( req->headers().keyHasValue("command", "quit") ) {
                qDebug("\n\na quit has been requested!\n");
                QCoreApplication::instance()->quit();
            }
        });
    }

    virtual ~ClientHandler() {
        qDebug("  ~ClientHandler(#%llu): I've being called automatically!",
                iconnectionId
                );
    }

protected:
    quint64    iconnectionId;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace anon
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);
#if defined(Q_OS_UNIX)
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
#endif

    // dumb (trivial) connection counter
    quint64 iconnectionCounter = 0;

    QString portOrUnixSocket("10022"); // default: TCP port 10022
    if ( argc > 1 )
        portOrUnixSocket = argv[1];

    QHttpServer server(&app);
    server.listen(portOrUnixSocket, [&](QHttpRequest* req, QHttpResponse* res){
        new ClientHandler(++iconnectionCounter, req, res);
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
