#include "httpserver.hpp"

#include "qhttprequest.hpp"
#include "qhttpresponse.hpp"

#include <QtCore/QDateTime>
///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
HttpServer::HttpServer(QObject *parent) : QHttpServer(parent), icounter(0) {
}

HttpServer::~HttpServer() {
}

void
HttpServer::incomingRequest(QHttpRequest *req, QHttpResponse *resp) {

    qDebug("a new request (#%d) is comming from %s:%d",
           icounter,
           req->remoteAddress().toUtf8().constData(),
           req->remotePort());

    QString body = QString("Hello World\n    packet count = %1\n    %2")
                   .arg(++icounter)
                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"));

    resp->setHeader("content-length", QString::number(body.size()).toLatin1());
    resp->writeHead(200);
    resp->write(body.toUtf8());

    ClientConnection* cc = new ClientConnection(req, resp, this);
    QObject::connect(cc,       &ClientConnection::requestQuit,
                     [this](){
        qDebug("close the server because of a HTTP quit request.");
        emit quit();
    });
}

///////////////////////////////////////////////////////////////////////////////
ClientConnection::ClientConnection(QHttpRequest *req, QHttpResponse *resp, QObject* p) :
    QObject(p), ireq(req), iresp(resp) {

    QObject::connect(req,        &QHttpRequest::data,
                     [this](const QByteArray& chunk){
        ibody.append(chunk);
    });

    QObject::connect(req,       &QHttpRequest::end,
                     this,     &ClientConnection::onComplete
                     );
}

void
ClientConnection::onComplete() {
    static const char KContentType[]        = "content-type";
    static const char KCommand[]            = "command";

    if ( ireq->method() == QHttpRequest::HTTP_POST ) {
        qDebug("path of POST request: %s", qPrintable(ireq->path()));

        const THeaderHash &headers = ireq->headers();

        if ( headers.contains(KCommand) ) {
            const QByteArray& value = headers.value(KCommand);
            if ( value == "quit" ) {
                qDebug("a quit has been requested!");
                emit requestQuit();
            }

        } else if ( headers.contains(KContentType) ) {
            const QByteArray& value = headers.value(KContentType);
            if ( value == "application/json" ) {

                qDebug("body:\n%s", ibody.constData());
            }
        }
    }


    qDebug("end of client connection");
    iresp->end();
    ireq->deleteLater();
    deleteLater();
}

///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////

