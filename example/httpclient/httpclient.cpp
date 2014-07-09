#include "httpclient.hpp"
#include "include/jsonbuilder.hpp"
#include "include/gason.hpp"

#include "private/qhttpclient_private.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"

#include <QTimerEvent>
#include <QBasicTimer>

#include <QTcpSocket>
#include <QHostAddress>
///////////////////////////////////////////////////////////////////////////////
class HttpClientPrivate : public qhttp::client::QHttpClientPrivate
{
    Q_DECLARE_PUBLIC(HttpClient)

public:
    const int              iclientId;

    quint32                isleep;
    quint32                icommandCounter;
    quint32                irequests;

    QBasicTimer            itimer;
    QString                iaddress;
    quint16                iport;

    bool                   ikeepAlive;

    gason::JsonAllocator   ijsonAllocator;
    QByteArray             ibuffer;

public:
    explicit    HttpClientPrivate(int clientId, HttpClient* q)
        : qhttp::client::QHttpClientPrivate(q),
          iclientId(clientId) {
    }

    void        initialize() {
        isleep          = 0;
        icommandCounter = 0;
        irequests       = 0;

        QObject::connect(q_func(), &qhttp::client::QHttpClient::disconnected, [this](){
            if ( isleep == 0 )
                QMetaObject::invokeMethod(q_func(), "start");
        });
    }

public:
    bool        shouldSend() {
        if ( icommandCounter < irequests )
            return true;

        itimer.stop();
        q_func()->close();
        emit q_func()->finished(iclientId, icommandCounter);
        return false;
    }

    void        requestStart() {
        if ( !shouldSend() )
            return;

        if ( !q_func()->isOpen() ) {
            QUrl url;
            url.setScheme("http");
            url.setHost(iaddress);
            url.setPort(iport);
            url.setPath("/aPath/users");

            q_func()->request(qhttp::EHTTP_POST, url);
        }
    }

protected:
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
};


///////////////////////////////////////////////////////////////////////////////

HttpClient::HttpClient(quint32 clientId, QObject *parent)
    : qhttp::client::QHttpClient(*new HttpClientPrivate(clientId, this), parent) {
    d_func()->initialize();
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
    if ( sleep < 10000 )
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
    d_func()->requestStart();
}

void
HttpClient::timerEvent(QTimerEvent* e) {
    Q_D(HttpClient);
    if ( e->timerId() == d->itimer.timerId() ) {
        QMetaObject::invokeMethod(this, "start");
    }

    qhttp::client::QHttpClient::timerEvent(e);
}

void
HttpClient::onRequestReady(qhttp::client::QHttpRequest *req) {
    Q_D(HttpClient);

    char requestData[257] = {0};
    gason::JSonBuilder json(requestData, 256);
    json.startObject()
            .addValue("clientId", d->iclientId)
            .addValue("requestId", (int)d->icommandCounter++)
            .addValue("command", "request")
        .endObject();

    req->addHeader("connection", (d->ikeepAlive) ? "keep-alive" : "close");
    req->addHeader("content-length", QByteArray::number((int)strlen(requestData)));

    req->end(requestData);

    if ( d->isleep > 0 )
        d->itimer.start(d->isleep, this);
}

void
HttpClient::onResponseReady(qhttp::client::QHttpResponse *res) {
    Q_D(HttpClient);

    d->ibuffer.clear();
    d->ibuffer.reserve(1024);

    QObject::connect(res, &qhttp::client::QHttpResponse::data,
                     [this, d](const QByteArray& chunk){
        d->ibuffer.append(chunk);
        puts("data chunk:");
        puts(chunk.constData());
    });

    QObject::connect(res, &qhttp::client::QHttpResponse::end,
                     [this, d](){
        printf("got the response: clientId=%d, responseId=%d\n",
               d->iclientId, d->icommandCounter);
        d->onIncomming();
        close();
    });
}
