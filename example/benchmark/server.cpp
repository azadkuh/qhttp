#include "server.hpp"

#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include "../include/gason.hpp"
#include "../include/jsonbuilder.hpp"

#include <QCoreApplication>
#include <QTimerEvent>
#include <QBasicTimer>
#include <QElapsedTimer>
#include <QLocale>
#include <QDateTime>

using namespace qhttp::server;
///////////////////////////////////////////////////////////////////////////////

class ClientHandler : public QObject
{
public:
    explicit        ClientHandler(QHttpRequest*  req, QHttpResponse* res) : QObject(req->connection()) {

        QObject::connect(req, &QHttpRequest::data, [this, req](const QByteArray& chunk) {
            // data attack!
            if ( ibody.size() > 1024 )
                req->connection()->close();
            else
                ibody.append(chunk);
        });

        QObject::connect(req, &QHttpRequest::end, [this, req, res](){
            res->addHeader("connection", "close");

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

            if ( req->headers().keyHasValue("command", "quit" ) ) {
                puts("a quit header is received!");

                QCoreApplication::instance()->quit();
            }
        });

        ibody.reserve(1024);
    }

protected:
    QByteArray      ibody;
};

///////////////////////////////////////////////////////////////////////////////

class ServerPrivate
{
    Q_DECLARE_PUBLIC(Server)
    Server* const   q_ptr;

public:
    QBasicTimer     itimer;
    QElapsedTimer   ielapsed;

    quint64         itotalHandled;      ///< total connections being handled.
    quint32         itempHandled;       ///< connections handled in interval time

public:
    explicit    ServerPrivate(Server* q) : q_ptr(q) {
        itotalHandled   = 0;
        itempHandled    = 0;
    }


    void        log() {
        itotalHandled   += itempHandled;
        quint32 miliSec  = (quint32) ielapsed.elapsed();
        float   aveTps   = float(itempHandled * 1000.0) / float(miliSec);
        QString dateTime = QLocale::c().toString(
                               QDateTime::currentDateTime(),
                               "yyyy-MM-dd hh:mm:ss");

        printf("%s,%.1f,%u,%u,%llu\n",
               qPrintable(dateTime),
               aveTps, miliSec,
               itempHandled, itotalHandled
               );

        fflush(stdout);

        itempHandled = 0;
        ielapsed.start();
    }

};

///////////////////////////////////////////////////////////////////////////////

Server::Server(QObject *parent) : QHttpServer(parent), d_ptr(new ServerPrivate(this)) {
    Q_D(Server);


    printf("\nDataTime,AveTps,miliSecond,Count,TotalCount\n");
    d->itimer.start(10000, Qt::CoarseTimer, this);
    d->ielapsed.start();
}

Server::~Server() {
}

void
Server::incomingConnection(QHttpConnection *connection) {
    QObject::connect(connection, &QHttpConnection::newRequest,
                     [this](QHttpRequest* req, QHttpResponse* res){
        new ClientHandler(req, res);
    });

    QObject::connect(connection, &QHttpConnection::disconnected, [this](){
        Q_D(Server);

        d->itempHandled++;
    });
}

void
Server::timerEvent(QTimerEvent *e) {
    Q_D(Server);

    if ( e->timerId() == d->itimer.timerId() ) {
        d->log();
    }

    QHttpServer::timerEvent(e);
}

///////////////////////////////////////////////////////////////////////////////


