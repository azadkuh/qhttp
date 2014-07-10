#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include <QtCore/QCoreApplication>
#include <QDateTime>
#include <QLocale>
///////////////////////////////////////////////////////////////////////////////

/** connection class for gathering incomming chunks of data from HTTP client. */
class ClientHandler : public QObject
{
public:
    explicit        ClientHandler(quint64 id, qhttp::server::QHttpConnection* conn)
        : QObject(conn), iconnectionId(id) {

        // auto delete this after connection closes or drops.
        QObject::connect(conn, &qhttp::server::QHttpConnection::disconnected, [this](){
            deleteLater();
        });

        // process incomming connection.
        QObject::connect(conn, &qhttp::server::QHttpConnection::newRequest,
                         [this](qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res){
            processRequest(req, res);
        });

    }

    virtual        ~ClientHandler() {
    }

    void            processRequest(qhttp::server::QHttpRequest* req,
                                   qhttp::server::QHttpResponse* res) {

        // append chunks of data into uniform body.
        QObject::connect(req, &qhttp::server::QHttpRequest::data, [this](const QByteArray& chunk){
            ibody.append(chunk);
        });

        // when all the incoming data are gathered, send some response to client.
        QObject::connect(req, &qhttp::server::QHttpRequest::end, [this, req, res](){
            printf("connection (#%llu): request from %s:%d\nurl: %s\n",
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
            res->addHeader("content-length", QString::number(body.size()).toLatin1());
            res->end(body.toUtf8());

            if ( req->headers().keyHasValue("command", "quit") ) {
                printf("a quit has been requested!\n");
                QCoreApplication::instance()->quit();
            }
        });
    }

protected:
    quint64         iconnectionId;
    QByteArray      ibody;
};

///////////////////////////////////////////////////////////////////////////////

class HttpServer : public qhttp::server::QHttpServer
{
    quint64     iconnectionCounter;

public:
    explicit    HttpServer(QObject* parent)
        : qhttp::server::QHttpServer(parent), iconnectionCounter(0) {
    }

    virtual    ~HttpServer() {
    }

protected:
    void        incomingConnection(qhttp::server::QHttpConnection* connection) {
        // the instance will be automatically deleted when the connection closes.
        new ClientHandler(iconnectionCounter++, connection);
    }
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);

    HttpServer server(&app);
    server.listen(8080);

    app.exec();
}
