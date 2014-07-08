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
class HttpClient::Private
{
    static const size_t    KPacketMaxLength = 1024;
    HttpClient*            iparent;

public:
    qhttp::client::QHttpClient  ihttpClient;
    int                    iclientId;

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
    explicit    Private(int clientId, HttpClient* p) : iparent(p), ihttpClient(p) {
        iclientId       = clientId;
        isleep          = 0;
        icommandCounter = 0;
        irequests       = 0;

        QObject::connect(&ihttpClient, &qhttp::client::QHttpClient::connected,
                         [this](qhttp::client::QHttpRequest* req){
            onRequest(req);
        });

        QObject::connect(&ihttpClient, &qhttp::client::QHttpClient::newResponse,
                         [this](qhttp::client::QHttpResponse* res){
            onResponse(res);
        });

        QObject::connect(&ihttpClient, &qhttp::client::QHttpClient::disconnected, [this](){
            if ( isleep == 0 )
                QMetaObject::invokeMethod(iparent, "start");
        });
    }

public:
    bool        shouldSend() {
        if ( icommandCounter < irequests )
            return true;

        itimer.stop();
        ihttpClient.close();
        emit iparent->finished(iclientId, icommandCounter);
        return false;
    }

    void        requestStart() {
        if ( !shouldSend() )
            return;

        if ( !ihttpClient.isOpen() ) {
            QUrl url;
            url.setScheme("http");
            url.setHost(iaddress);
            url.setPort(iport);
            url.setPath("/aPath/users");

            ihttpClient.request(qhttp::EHTTP_POST, url);
        }
    }

    void        onRequest(qhttp::client::QHttpRequest* req) {
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

        if ( isleep > 0 )
            itimer.start(isleep, iparent);
    }

protected:
    void        onResponse(qhttp::client::QHttpResponse* res) {
        ibuffer.clear();
        ibuffer.reserve(1024);
        printf("got the response: clientId=%d, responseId=%d\n", iclientId, icommandCounter);

        QObject::connect(res, &qhttp::client::QHttpResponse::data,
                         [this](const QByteArray& chunk){
            ibuffer.append(chunk);
        });

        QObject::connect(res, &qhttp::client::QHttpResponse::end, [this](){
            onIncomming();
            ihttpClient.close();
        });
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

HttpClient::HttpClient(quint32 clientId, QObject *parent) : QObject(parent), pimp(nullptr) {

    pimp                = new Private(clientId, this);
}

HttpClient::~HttpClient() {
    if ( pimp != nullptr ) {
        delete pimp;
        pimp = nullptr;
    }
}

quint32
HttpClient::clientId() const {
    return pimp->iclientId;
}

const QString&
HttpClient::address() const {
    return pimp->iaddress;
}

void
HttpClient::setAddress(const QString& addr) {
    pimp->iaddress = addr;
}

quint16
HttpClient::port() const {
    return pimp->iport;
}

void
HttpClient::setPort(quint16 p) {
    pimp->iport = p;
}

quint32
HttpClient::requestCount() const {
    return pimp->irequests;
}

void
HttpClient::setRequestCount(quint32 rc) {
    pimp->irequests = rc;
}

quint32
HttpClient::sleepTime() const {
    return pimp->isleep;
}

void
HttpClient::setSleepTime(quint32 sleep) {
    if ( sleep < 10000 )
        pimp->isleep = sleep;
    else
        pimp->isleep = 100;
}

quint32
HttpClient::commandCount() const {
    return pimp->icommandCounter;
}

bool
HttpClient::keepAlive() const {
    return pimp->ikeepAlive;
}

void
HttpClient::setKeepAlive(bool b) {
    pimp->ikeepAlive = b;
}

void
HttpClient::start() {
    Private& d = *pimp;

    d.requestStart();
}

void
HttpClient::timerEvent(QTimerEvent* e) {
    if ( pimp != nullptr ) {
        if ( e->timerId() == pimp->itimer.timerId() )
            QMetaObject::invokeMethod(this, "start");
    }
}
///////////////////////////////////////////////////////////////////////////////

