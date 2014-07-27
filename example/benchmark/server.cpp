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
#include <QThread>

using namespace qhttp::server;
///////////////////////////////////////////////////////////////////////////////

class ClientHandler : public QHttpConnection
{
public:
    explicit    ClientHandler(QThread* thread, qintptr sokDesc, qhttp::TBackend backend)
        : QHttpConnection(nullptr) {
        setSocketDescriptor(sokDesc, backend);

        moveToThread(thread);
        QObject::connect(thread, &QThread::finished, [this](){
            killConnection();
        });

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
    }

    virtual    ~ClientHandler() {
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

    static const size_t KThreadCount = 4;
    QThread             ithreads[KThreadCount];

public:
    explicit    ServerPrivate(Server* q) : q_ptr(q) {
        itotalHandled   = 0;
        itempHandled    = 0;
    }

    virtual    ~ServerPrivate() {
        for ( size_t i = 0;    i < KThreadCount;    i++ ) {
            ithreads[i].quit();
        }

        for ( size_t i = 0;    i < KThreadCount;    i++ ) {
            ithreads[i].wait(10000);
        }
    }

    void        start() {
        printf("\nDateTime,AveTps,miliSecond,Count,TotalCount\n");
        itimer.start(10000, Qt::CoarseTimer, q_ptr);
        ielapsed.start();

        for ( size_t i = 0;    i < KThreadCount;    i++ ) {
            ithreads[i].start();
        }

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
    d_func()->start();
}

Server::~Server() {
}

void
Server::incomingConnection(qintptr handle) {
    static quint64 counter = 0;

    QThread* th  = &d_func()->ithreads[counter % ServerPrivate::KThreadCount];
    counter++;

    ClientHandler* cli   = new ClientHandler(th, handle, qhttp::ETcpSocket);
    cli->setTimeOut(timeOut());

    QObject::connect(cli, &QHttpConnection::disconnected, [this](){
        d_func()->itempHandled++;
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


