#include "httpclient.hpp"
#include "include/jsonbuilder.hpp"
#include "include/gason.hpp"

#include <QTimerEvent>
#include <QBasicTimer>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
///////////////////////////////////////////////////////////////////////////////
class HttpClient::Private
{
    static const size_t    KPacketMaxLength = 4096;
    HttpClient*            iparent;

public:
    QNetworkAccessManager  inetworkManager;
    int                    iclientId;

    quint32                isleep;
    quint32                icommandCounter;
    quint32                irequests;

    QBasicTimer            itimer;
    QString                iaddress;
    quint16                iport;

    bool                   ikeepAlive;

    gason::JsonAllocator   ijsonAllocator;

public:
    explicit    Private(int clientId, HttpClient* p) : iparent(p), inetworkManager(p) {
        iclientId       = clientId;
        isleep          = 0;
        icommandCounter = 0;
        irequests       = 0;

        QObject::connect(&inetworkManager, &QNetworkAccessManager::finished, [this](QNetworkReply* rep){
            onFinished(rep);
        });
    }

public:
    void        sendRequest() {
        if ( icommandCounter >= irequests ) {
            emit iparent->finished(iclientId, icommandCounter);
            return;
        }

        QNetworkRequest req(QString("http://%1:%2/testPath")
                            .arg(iaddress)
                            .arg(iport)
                            );
        if ( !ikeepAlive )
            req.setRawHeader("connection", "close");
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        char buffer[KPacketMaxLength+1] = {0};
        gason::JSonBuilder doc(buffer, KPacketMaxLength);
        doc.startObject()
                .addValue("clientId", iclientId)
                .addValue("requestId", (int) icommandCounter++)
                .addValue("command", "request")
           .endObject();

        inetworkManager.post(req, QByteArray(buffer));
    }

    void        onFinished(QNetworkReply* reply) {
        char buffer[KPacketMaxLength+1] = {0};
        reply->read(buffer, KPacketMaxLength);

        reply->deleteLater();

        onIncomingData(buffer);

        if ( isleep > 0 )
            itimer.start(isleep, iparent);
        else
            QMetaObject::invokeMethod(iparent, "start");
    }

protected:
    bool        onIncomingData(char* buffer) {
        gason::JsonValue root;

        if ( gason::jsonParse(buffer, root, ijsonAllocator) != gason::JSON_PARSE_OK )
            return false;

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

    d.sendRequest();
}

void
HttpClient::timerEvent(QTimerEvent* e) {
    if ( pimp != nullptr ) {
        if ( e->timerId() == pimp->itimer.timerId() )
            pimp->sendRequest();
    }
}
///////////////////////////////////////////////////////////////////////////////

