#include "httpserver.hpp"

#include "qhttpconnection.hpp"
#include "qhttprequest.hpp"
#include "qhttpresponse.hpp"

#include <QtCore/QDateTime>
///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////
HttpServer::HttpServer(QObject *parent) : QHttpServer(parent), icounter(0) {
    QObject::connect(this, &HttpServer::newRequest, [this](QHttpRequest* req, QHttpResponse* resp){
        this->incomingRequest(req, resp);
    });
}

HttpServer::~HttpServer() {
}

void
HttpServer::incomingRequest(QHttpRequest* req, QHttpResponse* resp) {
    printf("a new request (#%d) is comming from %s:%d\n",
           ++icounter,
           req->remoteAddress().toUtf8().constData(),
           req->remotePort());

    QString body = QString("Hello World\n    packet count = %1\n    time = %2\n")
                   .arg(icounter)
                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    resp->setHeader("content-length", QString::number(body.size()).toLatin1());
    //resp->setHeader("connection", "close");
    resp->writeHead(200);
    resp->write(body.toUtf8());

    ClientConnection* cc = new ClientConnection(req, resp, this);
    QObject::connect(cc,       &ClientConnection::requestQuit,
                     this,     &HttpServer::closed,
                     Qt::QueuedConnection);
}


///////////////////////////////////////////////////////////////////////////////
ClientConnection::ClientConnection(QHttpRequest *req, QHttpResponse *resp, QObject* p) :
    QObject(p), ireq(req), iresp(resp) {

    QObject::connect(req, &QHttpRequest::data, [this](const QByteArray& chunk){
        ibody.append(chunk);
    });

    QObject::connect(req,      &QHttpRequest::end,
                     this,     &ClientConnection::onComplete
                     );
}

void
ClientConnection::onComplete() {
    const THeaderHash &headers = ireq->headers();

    if ( headers.value("command") == "quit" ) {
        printf("a quit has been requested!\n");
        emit requestQuit();
    }

    if ( ireq->method() == QHttpRequest::HTTP_POST )
        printf("body: \"%s\"\n", ibody.constData());


    printf("end of client connection\n");
    iresp->deleteLater();
    ireq->deleteLater();
    deleteLater();
}

///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////

