#include "httpserver.hpp"

#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include <QtCore/QDateTime>

#include "../include/gason.hpp"
#include "../include/jsonbuilder.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////

HttpServer::HttpServer(QObject *parent) : QHttpServer(parent) {
}

HttpServer::~HttpServer() {
}

void
HttpServer::incomingConnection(qhttp::server::QHttpConnection* conn) {
    ClientConnection* cc = new ClientConnection(conn);

    QObject::connect(cc, &ClientConnection::requestQuit, [this](){
        this->close();
        emit closed();
    });
}


///////////////////////////////////////////////////////////////////////////////

ClientConnection::ClientConnection(qhttp::server::QHttpConnection* conn)
    : QObject(conn) {

    QObject::connect(conn, &qhttp::server::QHttpConnection::disconnected, [this](){
        deleteLater();
    });

    QObject::connect(conn, &qhttp::server::QHttpConnection::newRequest,
                     this, &ClientConnection::processRequest
                     );
}

void
ClientConnection::processRequest(qhttp::server::QHttpRequest *req,
                                 qhttp::server::QHttpResponse *res) {

    QObject::connect(req, &qhttp::server::QHttpRequest::data,
                     [this](const QByteArray& chunk) {
        // data attack!
        if ( ibody.size() > 4096 )
            this->deleteLater();
        else
            ibody.append(chunk);
    });


    QObject::connect(req, &qhttp::server::QHttpRequest::end, [this, req, res](){
        res->addHeader("connection", "close");

        if ( req->headers().keyHasValue("command", "quit" ) ) {
            puts("a quit header is received!");

            res->setStatusCode(qhttp::ESTATUS_OK);
            res->end("server closed!\n");
            emit requestQuit();
            return;
        }

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

                res->addHeader("content-length", QByteArray::number((int)strlen(buffer)));

                clientStatus = true;
            }

        }

        if ( clientStatus ) {
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->end(QByteArray(buffer));

        } else {
            res->setStatusCode(qhttp::ESTATUS_BAD_REQUEST);
            res->end("bad request: the json value is not present or invalid!\n");
        }
    });
}


///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////

