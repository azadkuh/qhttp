#include "clients.hpp"

#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"

#include "../include/gason.hpp"
#include "../include/jsonbuilder.hpp"

#include <QCoreApplication>
#include <QTimerEvent>
#include <QBasicTimer>

using namespace qhttp::client;
///////////////////////////////////////////////////////////////////////////////

class Client : public QObject
{
public:
    quint32         itimeOut;
    quint16         iport;
    QString         iaddress;

protected:
    const quint32   iid;
    quint32         irequestId;

    QBasicTimer     itimer;

    QByteArray      ibuffer;
    gason::JsonAllocator ijsonAllocator;

public:
    explicit    Client(quint32 id, Clients* parent) : QObject(parent), iid(id) {
        irequestId = 0;
        ibuffer.reserve(1024);
    }

    virtual    ~Client() {
    }

    void        start() {
        itimer.start(itimeOut, Qt::CoarseTimer, this);
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

        QObject::connect(res, &QHttpResponse::data, [this](const QByteArray& chunk){
            ibuffer.append(chunk);
        });

        QObject::connect(res, &QHttpResponse::end, [this, res](){
            onBody();

            res->connection()->close();
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

protected:
    void        timerEvent(QTimerEvent *e) {
        if ( e->timerId() != itimer.timerId() )
            return;

        itimer.stop();

        QHttpClient* client = new QHttpClient(this);

        QObject::connect(client, &QHttpClient::disconnected, [this](){
            start();
        });
        QObject::connect(client, &QHttpClient::httpConnected, [this](QHttpRequest* req){
            onRequest(req);
        });
        QObject::connect(client, &QHttpClient::newResponse, [this](QHttpResponse* res){
            onResponse(res);
        });


        QUrl url;
        url.setScheme("http");
        url.setHost(iaddress);
        url.setPort(iport);
        url.setPath("/aPath");

        client->request(qhttp::EHTTP_POST, url);
    }

};

///////////////////////////////////////////////////////////////////////////////

class ClientsPrivate
{
    Q_DECLARE_PUBLIC(Clients)
    Clients* const  q_ptr;

public:
    explicit    ClientsPrivate(Clients* q) : q_ptr(q) {
    }
};

///////////////////////////////////////////////////////////////////////////////
Clients::Clients(QObject *parent) : QObject(parent), d_ptr(new ClientsPrivate(this)) {
}

Clients::~Clients() {
}

bool
Clients::setup(const QString &address, quint16 port, quint32 count, quint32 timeOut) {
    if ( timeOut == 0 )
        timeOut = 10;
    else if ( timeOut > 10000 )
        timeOut = 100;

    if ( count == 0 )
        return false;
    else if ( count > 1000 )
        count = 1000;


    for ( size_t i = 0;    i < count;    i++ ) {
        Client* cli = new Client(i+1, this);

        cli->iport      = port;
        cli->iaddress   = address;
        cli->itimeOut   = timeOut;

        cli->start();
    }

    return true;
}
