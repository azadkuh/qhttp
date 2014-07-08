#include "httpserver.hpp"

#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include <QtCore/QDateTime>
///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////
HttpServer::HttpServer(QObject *parent) : QHttpServer(parent), icounter(0) {
}

HttpServer::~HttpServer() {
}

void
HttpServer::incomingConnection(qhttp::server::QHttpConnection *connection) {
    ClientConnection* cc = new ClientConnection(icounter++, connection);

    QObject::connect(cc,        &ClientConnection::requestQuit, [this](){
        this->close();
        emit closed();
    });
}

///////////////////////////////////////////////////////////////////////////////
ClientConnection::ClientConnection(uint32_t id, qhttp::server::QHttpConnection* conn)
    : QObject(conn), iconnectionId(id) {

    QObject::connect(conn, &qhttp::server::QHttpConnection::disconnected, [this](){
        deleteLater();
    });

    QObject::connect(conn, &qhttp::server::QHttpConnection::newRequest,
                     [this](qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res){
        processRequest(req, res);
    });
}

void
ClientConnection::processRequest(qhttp::server::QHttpRequest* req,
                                 qhttp::server::QHttpResponse* res) {

    QObject::connect(req, &qhttp::server::QHttpRequest::data, [this](const QByteArray& chunk){
        ibody.append(chunk);
    });

    QObject::connect(req, &qhttp::server::QHttpRequest::end, [this, req, res](){
        printf("connection (#%d), a new request is comming from %s:%d\nurl: %s\n",
               iconnectionId,
               req->remoteAddress().toUtf8().constData(),
               req->remotePort(),
               qPrintable(req->url().toString())
               );

        if ( req->method() == qhttp::EHTTP_POST )
            printf("body: \"%s\"\n", ibody.constData());

        QString body = QString("Hello World\n    packet count = %1\n    time = %2\n")
                       .arg(iconnectionId)
                       .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

        res->setStatusCode(qhttp::ESTATUS_OK);
        res->addHeader("content-length", QString::number(body.size()).toLatin1());
        res->end(body.toUtf8());

        if ( req->headers().keyHasValue("command", "quit") ) {
            printf("a quit has been requested!\n");
            emit requestQuit();
        }
    });
}


///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////

