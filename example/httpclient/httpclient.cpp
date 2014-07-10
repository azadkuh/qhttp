#define  QHTTP_MEMORY_LOG 0
#include "httpclient.hpp"
#include "include/jsonbuilder.hpp"
#include "include/gason.hpp"

#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"

#include <QTimerEvent>
#include <QBasicTimer>

#include <QTcpSocket>
#include <QHostAddress>

///////////////////////////////////////////////////////////////////////////////
class HttpClientPrivate
{
    HttpClient* const      q_ptr;

public:
    const int              iclientId;

    quint32                isleep;
    quint32                icommandCounter;
    quint32                irequests;

    QBasicTimer            isleepTimer;
    QString                iaddress;
    quint16                iport;

    bool                   ikeepAlive;

    gason::JsonAllocator   ijsonAllocator;
    QByteArray             ibuffer;

public:
    explicit    HttpClientPrivate(int clientId, HttpClient* q)
        : q_ptr(q), iclientId(clientId) {

        isleep          = 0;
        icommandCounter = 0;
        irequests       = 0;

        QHTTP_LINE_LOG
    }

    virtual    ~HttpClientPrivate() {
        QHTTP_LINE_LOG
    }

public:
    bool        shouldSend() {
        if ( icommandCounter < irequests )
            return true;

        isleepTimer.stop();
        emit q_ptr->finished(iclientId, icommandCounter);
        return false;
    }

    void        requestStart() {
        if ( !shouldSend() )
            return;

        qhttp::client::QHttpClient* client = new qhttp::client::QHttpClient(q_ptr);

        QObject::connect(client,    &qhttp::client::QHttpClient::disconnected,
                         [this](){
            QMetaObject::invokeMethod(q_ptr, "start");
        });
        QObject::connect(client,    &qhttp::client::QHttpClient::httpConnected,
                         [this](qhttp::client::QHttpRequest* req){
            onRequestReady(req);
        });
        QObject::connect(client,    &qhttp::client::QHttpClient::newResponse,
                         [this](qhttp::client::QHttpResponse* res){
            onResponseReady(res);
        });


        QUrl url;
        url.setScheme("http");
        url.setHost(iaddress);
        url.setPort(iport);
        url.setPath("/aPath/users");

        client->request(qhttp::EHTTP_POST, url);
    }

    bool        onIncomming() {
        char buffer[1025] = {0};
        strncpy(buffer, ibuffer.constData(), 1024);

        gason::JsonValue root;

        if ( gason::jsonParse(buffer, root, ijsonAllocator) != gason::JSON_PARSE_OK ) {
            puts("invalid json response, parsing failed.");
            return false;
        }

        gason::JsonValue clientId   = root("clientId");
        gason::JsonValue reqId      = root("requestId");
        gason::JsonValue command    = root("command");

        bool bok = false;

        if ( strncmp("response", command.toString(&bok), 8) != 0 ) {
            puts("    invalid command!\n");
            return false;
        }

        if ( !clientId.isNumber()    ||    clientId.toInt(&bok) != iclientId ) {
            puts("    invalid clientId!\n");
            return false;
        }

        if ( !reqId.isNumber()    ||    reqId.toInt(&bok) != (int)icommandCounter ) {
            puts("    invalid requestId!\n");
            return false;
        }

        return true;
    }

    void        onRequestReady(qhttp::client::QHttpRequest *req) {
        char requestData[257] = {0};
        gason::JSonBuilder json(requestData, 256);
        json.startObject()
                .addValue("clientId", iclientId)
                .addValue("requestId", (int)icommandCounter++)
                .addValue("command", "request")
            .endObject();

        req->addHeader("connection", (ikeepAlive) ? "keep-alive" : "close");
        req->addHeader("content-length", QByteArray::number((int)strlen(requestData)));

        req->end(requestData);
    }

    void        onResponseReady(qhttp::client::QHttpResponse *res) {
        ibuffer.clear();
        ibuffer.reserve(1024);

        QObject::connect(res, &qhttp::client::QHttpResponse::data,
                         [this](const QByteArray& chunk){
            ibuffer.append(chunk);
        });

        QObject::connect(res, &qhttp::client::QHttpResponse::end,
                         [this, res](){
            onIncomming();
            res->releaseConnection();
        });
    }

};


///////////////////////////////////////////////////////////////////////////////

HttpClient::HttpClient(quint32 clientId, QObject *parent) : QObject(parent),
   d_ptr(new HttpClientPrivate(clientId, this)) {
}

HttpClient::~HttpClient() {
}

quint32
HttpClient::clientId() const {
    return d_func()->iclientId;
}

const QString&
HttpClient::address() const {
    return d_func()->iaddress;
}

void
HttpClient::setAddress(const QString& addr) {
    d_func()->iaddress = addr;
}

quint16
HttpClient::port() const {
    return d_func()->iport;
}

void
HttpClient::setPort(quint16 p) {
    d_func()->iport = p;
}

quint32
HttpClient::requestCount() const {
    return d_func()->irequests;
}

void
HttpClient::setRequestCount(quint32 rc) {
    d_func()->irequests = rc;
}

quint32
HttpClient::sleepTime() const {
    return d_func()->isleep;
}

void
HttpClient::setSleepTime(quint32 sleep) {
    if ( sleep < 100000 )
        d_func()->isleep = sleep;
    else
        d_func()->isleep = 100;
}

quint32
HttpClient::commandCount() const {
    return d_func()->icommandCounter;
}

bool
HttpClient::keepAlive() const {
    return d_func()->ikeepAlive;
}

void
HttpClient::setKeepAlive(bool b) {
    d_func()->ikeepAlive = b;
}

void
HttpClient::start() {
    Q_D(HttpClient);

    if ( d->isleepTimer.isActive() )
        return;

    if ( d->isleep > 0 )
        d->isleepTimer.start(d->isleep, Qt::CoarseTimer, this);
    else
        d->requestStart();
}

void
HttpClient::timerEvent(QTimerEvent* e) {
    Q_D(HttpClient);
    if ( e->timerId() == d->isleepTimer.timerId() )
        d->requestStart();
}
