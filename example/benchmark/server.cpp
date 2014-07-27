#include "server.hpp"

#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include "../include/gason.hpp"
#include "../include/jsonbuilder.hpp"
#include "../include/threadlist.hxx"

#include <QCoreApplication>
#include <QTimerEvent>
#include <QBasicTimer>
#include <QElapsedTimer>
#include <QLocale>
#include <QDateTime>


using namespace qhttp::server;
QAtomicInt  gHandledConnections;
///////////////////////////////////////////////////////////////////////////////

class ClientConnection : public QHttpConnection
{
public:
    explicit    ClientConnection(qintptr sokDesc, qhttp::TBackend backend, QObject* parent)
        : QHttpConnection(parent) {
        ibody.reserve(1024);

        onHandler([this](QHttpRequest*  req, QHttpResponse* res) {

            req->onData([this, req](const QByteArray& chunk) {
                // data attack!
                if ( ibody.size() > 1024 )
                    req->connection()->killConnection();
                else
                    ibody.append(chunk);
            });

            req->onEnd([this, req, res](){
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

        });

        setSocketDescriptor(sokDesc, backend);
    }

    virtual    ~ClientConnection() {
    }

protected:
    QByteArray        ibody;
};

class ClientHandler : public QObject
{
    Q_OBJECT

public:
    explicit    ClientHandler() {
    }

    void        setup(QThread* th) {
        moveToThread(th);

        QObject::connect(th,    &QThread::finished,    [this](){
            deleteLater();
        });
    }

public slots:
    void        start(int sokDesc, int bend) {
        qhttp::TBackend backend = static_cast<qhttp::TBackend>(bend);
        ClientConnection* cli = new ClientConnection((qintptr)sokDesc, backend, this);

        QObject::connect(cli,    &QHttpConnection::disconnected,    [this](){
            gHandledConnections.ref();
        });
    }
};

///////////////////////////////////////////////////////////////////////////////

class ServerPrivate
{
    Q_DECLARE_PUBLIC(Server)
    Server* const   q_ptr;

public:
    QBasicTimer     itimer;
    QElapsedTimer   ielapsed;

    quint64         itotalHandled = 0;   ///< total connections being handled.

#   if USETHREADS > 0
    static const size_t        KThreadCount = 4;
    ThreadList<KThreadCount>   ithreads;
    ClientHandler              iclients[KThreadCount];
#   endif

public:
    explicit    ServerPrivate(Server* q) : q_ptr(q) {
    }

    virtual    ~ServerPrivate() {
    }

    void        start() {
        printf("\nDateTime,AveTps,miliSecond,Count,TotalCount\n");
        itimer.start(10000, Qt::CoarseTimer, q_ptr);
        ielapsed.start();

#       if USETHREADS > 0
        for ( size_t i = 0;    i < KThreadCount;    i++ ) {
            iclients[i].setup( ithreads.at(i) );
        }
        ithreads.startAll();

#       endif

    }


    void        log() {
        quint32  tempHandled = gHandledConnections.load();

        itotalHandled   += tempHandled;
        quint32 miliSec  = (quint32) ielapsed.elapsed();
        float   aveTps   = float(tempHandled * 1000.0) / float(miliSec);
        QString dateTime = QLocale::c().toString(
                               QDateTime::currentDateTime(),
                               "yyyy-MM-dd hh:mm:ss");

        printf("%s,%.1f,%u,%u,%llu\n",
               qPrintable(dateTime),
               aveTps, miliSec,
               tempHandled, itotalHandled
               );

        fflush(stdout);

        gHandledConnections.store(0);
        ielapsed.start();
    }
};

///////////////////////////////////////////////////////////////////////////////
Server::Server(QObject *parent) : QHttpServer(parent), d_ptr(new ServerPrivate(this)) {
    d_func()->start();
}

Server::~Server() {
    stopListening();
}

void
Server::incomingConnection(qintptr handle) {
#   if USETHREADS > 0
    static quint64 counter = 0;

    size_t index = counter % ServerPrivate::KThreadCount;
    counter++;

    QMetaObject::invokeMethod(&d_func()->iclients[index],
                              "start",
                              Qt::QueuedConnection,
                              Q_ARG(int, handle),
                              Q_ARG(int, backendType())
                              );

#   else
    ClientConnection* cli = new ClientConnection(handle, backendType(), this);
    QObject::connect(cli,    &QHttpConnection::disconnected,    [](){
        gHandledConnections.ref();
    });

#   endif
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
#include "server.moc"

