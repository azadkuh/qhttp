#include "httpserver.hpp"

#include "qhttpconnection.hpp"
#include "qhttprequest.hpp"
#include "qhttpresponse.hpp"

#include <QtCore/QDateTime>

#include "../include/gason.hpp"
#include "../include/jsonbuilder.hpp"

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
    ClientConnection* cc = new ClientConnection(req, resp, this);
    QObject::connect(cc,       &ClientConnection::requestQuit,
                     this,     &HttpServer::closed,
                     Qt::QueuedConnection);
}


///////////////////////////////////////////////////////////////////////////////

ClientConnection::ClientConnection(QHttpRequest *req, QHttpResponse *resp, QObject* p) :
    QObject(p), ireq(req), iresp(resp) {

    QObject::connect(req, &QHttpRequest::data, [this](const QByteArray& chunk) {
        // data attack!
        if ( ibody.size() > 4096 )
            this->deleteLater();
        else
            ibody.append(chunk);
    });

    QObject::connect(req,      &QHttpRequest::end,
                     this,     &ClientConnection::onComplete
                     );
}

ClientConnection::~ClientConnection() {
    ireq->deleteLater();
    iresp->deleteLater();
}

void
ClientConnection::onComplete() {
    iresp->setHeader("connection", "close");

    // gason++ writes lots of \0 into source buffer. so we have to make a writeable copy.
    char buffer[4907] = {0};
    strncpy(buffer, ibody.constData(), 4096);

    gason::JsonAllocator    allocator;
    gason::JsonValue        root;

    bool  clientStatus = false;

    if ( gason::jsonParse(buffer, root, allocator) == gason::JSON_PARSE_OK ) {
        gason::JsonValue command   = root("command");
        gason::JsonValue clientId  = root("clientId");
        gason::JsonValue requestId = root("requestId");

        bool ok = false;
        if ( strncmp(command.toString(&ok), "request", 7) == 0  &&
             clientId.isNumber()    &&    requestId.isNumber() ) {

            memset(buffer, 0, 4096);
            gason::JSonBuilder doc(buffer, 4096);
            doc.startObject()
                    .addValue("command", "response")
                    .addValue("clientId", clientId.toInt(&ok))
                    .addValue("requestId", requestId.toInt(&ok) + 1)
               .endObject();

            clientStatus = true;
        }

    }

    if ( clientStatus ) {
        iresp->writeHead(QHttpResponse::STATUS_OK);
        iresp->end(QByteArray(buffer));

    } else {
        iresp->writeHead(QHttpResponse::STATUS_BAD_REQUEST);
        iresp->write("bad request: the json value is not present or invalid!\n");

    }

    deleteLater();
}

///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////

