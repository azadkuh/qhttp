#include "httpserver.hpp"

#include "qhttprequest.hpp"
#include "qhttpresponse.hpp"

#include <QtCore/QDateTime>
///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
HttpServer::HttpServer(QObject *parent) : QHttpServer(parent) {
}

HttpServer::~HttpServer() {
}

bool
HttpServer::initialize() {
    icounter = 0;
    QObject::connect(
                this,     SIGNAL(newRequest(QHttpRequest*,QHttpResponse*)),
                this,     SLOT(onRequest(QHttpRequest*,QHttpResponse*))
                );


    return false;
}

void
HttpServer::onRequest(QHttpRequest *req, QHttpResponse *resp) {

    qDebug("a new request (#%d) is comming from %s:%d",
           icounter,
           req->remoteAddress().toUtf8().constData(),
           req->remotePort());

    QString body = QString("Hello World\n    packet count = %1\n    %2")
                   .arg(++icounter)
                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz"));

    resp->setHeader("Content-Length", QString::number(body.size()));
    resp->writeHead(200);
    resp->write(body.toUtf8());

    ClientConnection* cc = new ClientConnection(req, resp, this);
    QObject::connect(
                cc,       SIGNAL(requestQuit()),
                this,     SLOT(onQuit()),
                Qt::QueuedConnection
                );
}

void
HttpServer::onQuit() {
    qDebug("close the server because of a HTTP quit request.");
    close();
    emit quit();
}

///////////////////////////////////////////////////////////////////////////////
ClientConnection::ClientConnection(QHttpRequest *req, QHttpResponse *resp, QObject* p) :
    QObject(p), ireq(req), iresp(resp) {

    QObject::connect(
                ireq,     SIGNAL(data(QByteArray)),
                this,     SLOT(appendData(QByteArray))
                );
    QObject::connect(
                ireq,     SIGNAL(end()),
                this,     SLOT(onComplete())
                );
}

void
ClientConnection::appendData(const QByteArray &arr) {
    ibody.append(arr);
}

void
ClientConnection::onComplete() {
    static const char KContentType[]        = "content-type";
    static const char KAppJSon[]            = "application/json";

    if ( ireq->method() == QHttpRequest::HTTP_POST ) {
        qDebug("path of POST request: %s", qPrintable(ireq->path()));

        const HeaderHash &headers = ireq->headers();
        if ( headers.contains(KContentType) ) {
            if ( headers.value(KContentType) == QLatin1String(KAppJSon) ) {

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

