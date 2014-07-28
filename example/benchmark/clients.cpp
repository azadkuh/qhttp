#include "clients.hpp"
#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"

#include "../include/gason.hpp"
#include "../include/jsonbuilder.hpp"
#include "../include/threadlist.hxx"

#include <QCoreApplication>
#include <QTimerEvent>
#include <QBasicTimer>

using namespace qhttp::client;
///////////////////////////////////////////////////////////////////////////////

class Client : public QObject
{
    Q_OBJECT

public:
    quint32         itimeOut;

    qhttp::TBackend ibackend;
    quint16         iport;
    QString         iaddress;

protected:
    const quint32   iid;
    quint32         irequestId = 0;

    QBasicTimer     itimer;

    QByteArray      ibuffer;
    gason::JsonAllocator ijsonAllocator;

public:
    explicit    Client(quint32 id, QObject* parent) : QObject(parent), iid(id) {
        ibuffer.reserve(1024);
    }

    virtual    ~Client() {
    }

    void        setup(QThread* thread) {
        moveToThread(thread);

        QObject::connect(thread, &QThread::started, [this](){
            QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
        });

        QObject::connect(thread, &QThread::finished, [this](){
            this->deleteLater();
        });
    }

    void        onRequest(QHttpRequest* req) {
        char requestData[257] = {0};
        gason::JSonBuilder json(requestData, 256);
        json.startObject()
                .addValue("clientId", (int)iid)
                .addValue("requestId", (int)irequestId++)
                .addValue("command", "request")
            .endObject();

        req->addHeader("connection", "close");
        req->addHeader("content-length", QByteArray::number((int)strlen(requestData)));

        req->end(requestData);

        if ( irequestId >= 2000000000 )
            irequestId = 0;
    }

    void        onResponse(QHttpResponse* res) {
        ibuffer.clear();

        res->onData([this](const QByteArray& chunk){
            ibuffer.append(chunk);
        });

        res->onEnd([this, res](){
            onBody();

            res->connection()->killConnection();
        });
    }

    bool        onBody() {
        char buffer[1025] = {0};
        strncpy(buffer, ibuffer.constData(), 1024);

        gason::JsonValue root;

        if ( gason::jsonParse(buffer, root, ijsonAllocator) != gason::JSON_PARSE_OK ) {
            fprintf(stderr, "invalid json response, parsing failed. id=%u, request=%u\n",
                    iid, irequestId);
            return false;
        }

        gason::JsonValue jclientId   = root("clientId");
        gason::JsonValue jreqId      = root("requestId");
        gason::JsonValue jcommand    = root("command");

        bool bok = false;

        const char* command = jcommand.toString(&bok);
        if ( bok == false    ||    strncmp("response", command, 8) != 0 ) {
            fprintf(stderr, "    invalid command! id=%u, command=%s\n",
                    iid, command);
            return false;
        }

        quint32 cid = jclientId.toInt(&bok);
        if ( bok == false    ||    cid != iid ) {
            fprintf(stderr,"    invalid clientId!\n id=%u, incoming id=%d\n",
                    iid, cid);
            return false;
        }

        quint32 reqId = jreqId.toInt(&bok);
        if ( bok == false    || reqId != irequestId  ) {
            fprintf(stderr, "    invalid requestId!, id=%u, requestId=%u, incomming requestId=%u\n",
                    iid, irequestId, reqId);
            return false;
        }

        return true;
    }

public slots:
    void        start() {
        itimer.start(itimeOut, Qt::CoarseTimer, this);
    }

protected:
    void        timerEvent(QTimerEvent *e) {
        if ( e->timerId() != itimer.timerId() )
            return;

        itimer.stop();

        QHttpClient* client = new QHttpClient(this);

        QUrl url;
        url.setHost(iaddress);

        if ( ibackend == qhttp::ELocalSocket ) {
            url.setScheme("socket");
        } else {
            url.setScheme("http");
            url.setPort(iport);
        }

        bool canRequest =  client->request(
                               qhttp::EHTTP_POST,
                               url,
                               [this](QHttpRequest* req) { onRequest(req);},
                               [this](QHttpResponse* res) { onResponse(res);});

        if ( canRequest ) {

            QObject::connect(client, &QHttpClient::disconnected, [this, client](){
                client->deleteLater();
                start();
            });

        } else
            client->deleteLater();

    }

};

///////////////////////////////////////////////////////////////////////////////

class ClientsPrivate
{
    Q_DECLARE_PUBLIC(Clients)
    Clients* const      q_ptr;

    ThreadList          ithreads;

public:
    explicit    ClientsPrivate(Clients* q) : q_ptr(q) {
    }

    void        setup(size_t threads) {
        if ( threads > 1 ) {
            ithreads.create(threads);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
Clients::Clients(size_t threads, QObject *parent) : QObject(parent),
    d_ptr(new ClientsPrivate(this)) {
    d_func()->setup(threads);
}

Clients::~Clients() {
}

bool
Clients::setup(qhttp::TBackend backend,
               const QString &address, quint16 port,
               quint32 count, quint32 timeOut) {
    Q_D(Clients);

    if ( timeOut == 0 )
        timeOut = 10;
    else if ( timeOut > 10000 )
        timeOut = 100;

    if ( count == 0 )
        return false;
    else if ( count > 1000 )
        count = 1000;


    for ( size_t i = 0;    i < count;    i++ ) {
        if ( d->ithreads.size() > 1 ) { // multi-thread
            Client* cli = new Client(i+1, nullptr);
            cli->ibackend   = backend;
            cli->iport      = port;
            cli->iaddress   = address;
            cli->itimeOut   = timeOut;

            QThread* th = d->ithreads.at(i);
            cli->setup(th);

        } else { // single-thread
            Client* cli = new Client(i+1, this);
            cli->ibackend   = backend;
            cli->iport      = port;
            cli->iaddress   = address;
            cli->itimeOut   = timeOut;

            cli->start();

        }
    }

    // in single thread mode, this line has no effect.
    d->ithreads.startAll();
    return true;
}
///////////////////////////////////////////////////////////////////////////////
#include "clients.moc"
