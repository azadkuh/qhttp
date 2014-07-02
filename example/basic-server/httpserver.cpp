#include "httpserver.hpp"

#include "qhttpconnection.hpp"
#include "qhttprequest.hpp"
#include "qhttpresponse.hpp"

#include <QtCore/QDateTime>
///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////
HttpServer::HttpServer(QObject *parent) : QHttpServer(parent), icounter(0) {
}

HttpServer::~HttpServer() {
}

void
HttpServer::incomingConnection(QHttpConnection *connection) {
    ClientConnection* cc = new ClientConnection(icounter++, connection);

    QObject::connect(cc,        &ClientConnection::requestQuit, [this](){
        this->close();
        emit closed();
    });
}

///////////////////////////////////////////////////////////////////////////////
ClientConnection::ClientConnection(uint32_t id, QHttpConnection* conn)
    : QObject(conn), iconnectionId(id) {

    QObject::connect(conn, &QHttpConnection::dropped, [this](){
        deleteLater();
    });

    QObject::connect(conn, &QHttpConnection::newRequest,
                     [this](QHttpRequest* req, QHttpResponse* res){
        processRequest(req, res);
    });
}

void
ClientConnection::processRequest(QHttpRequest* req, QHttpResponse* res) {

    QObject::connect(req, &QHttpRequest::data, [this](const QByteArray& chunk){
        ibody.append(chunk);
    });

    QObject::connect(req, &QHttpRequest::end, [this, req](){
        if ( req->method() == EHTTP_POST )
            printf("body: \"%s\"\n", ibody.constData());


        printf("end of client connection\n");

        const THeaderHash &headers = req->headers();

        if ( headers.value("command") == "quit" ) {
            printf("a quit has been requested!\n");
            emit requestQuit();
        }
    });

    printf("a new request (#%d) is comming from %s:%d\nurl: %s\n",
           iconnectionId,
           req->remoteAddress().toUtf8().constData(),
           req->remotePort(),
           qPrintable(req->url().toString())
           );

    QString body = QString("Hello World\n    packet count = %1\n    time = %2\n")
                   .arg(iconnectionId)
                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    res->setHeader("content-length", QString::number(body.size()).toLatin1());
    res->setHeader("connection", "close");
    res->writeHead(ESTATUS_OK);
    res->write(body.toUtf8());
}


///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////

