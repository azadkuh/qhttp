#include "httpclient.hpp"
#include "include/jsonbuilder.hpp"
#include "include/gason.hpp"

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
    QTcpSocket             isocket;
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
    explicit    Private(int clientId, HttpClient* p) : iparent(p), isocket(p) {
        iclientId       = clientId;
        isleep          = 0;
        icommandCounter = 0;
        irequests       = 0;

        QObject::connect(&isocket, &QTcpSocket::connected, [this](){
            QMetaObject::invokeMethod(iparent, "start");
        });
        QObject::connect(&isocket, &QTcpSocket::disconnected, [this](){
            isocket.close();
            QMetaObject::invokeMethod(iparent, "start");
        });
        QObject::connect(&isocket, &QTcpSocket::readyRead, [this](){
            if ( ibuffer.size() > (int) KPacketMaxLength ) {
                // buffer overflow!
                isocket.disconnectFromHost();

            } else {
                ibuffer.append( isocket.readAll() );
                onIncomingData();
            }
        });
    }

public:
    bool        shouldSend() {
        if ( icommandCounter < irequests )
            return true;

        isocket.disconnectFromHost();
        emit iparent->finished(iclientId, icommandCounter);
        return false;
    }

    void        requestStart() {
        if ( !shouldSend() )
            return;

        if ( !isocket.isOpen() ) {
            isocket.connectToHost(iaddress, iport);
            return;
        }

        ibuffer.clear();
        ibuffer.reserve(1024);

        char requestData[257] = {0};
        gason::JSonBuilder json(requestData, 256);
        json.startObject()
                .addValue("clientId", iclientId)
                .addValue("requestId", (int)icommandCounter++)
                .addValue("command", "request")
            .endObject();

        char requestHeader[257] = {0};
        gason::StringBuilder http(requestHeader, 256);

        http << "POST / HTTP/1.1\r\n"
             << "host: " << iaddress << ":" << (int)iport << "\r\n"
             << "connection: " << ( (ikeepAlive) ? "keep-alive" : "close" ) << "\r\n"
             << "content-length: " << (int)strlen(requestData) << "\r\n"
             << "\r\n";

        isocket.write(requestHeader);
        isocket.write(requestData);
        isocket.flush();
    }

protected:
    bool        onIncomingData() {
        int index = ibuffer.indexOf("\r\n\r\n");
        if ( index < 0 )
            return false;

        index += 4;
        char buffer[513] = {0};
        strncpy(buffer, ibuffer.constData() + index, 512);

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

