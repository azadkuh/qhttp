#include "private/qhttpserver_private.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////

QHttpServer::QHttpServer(QObject *parent)
    : QObject(parent), pPrivate(new QHttpServerPrivate) {
}

QHttpServer::QHttpServer(QHttpServerPrivate &dd, QObject *parent)
    : QObject(parent), pPrivate(&dd) {
}

QHttpServer::~QHttpServer() {
    stopListening();
}

bool
QHttpServer::listen(const QString &socketOrPort, const TServerHandler &handler) {
    bool isNumber   = false;
    quint16 tcpPort = socketOrPort.toUShort(&isNumber);
    if ( isNumber    &&    tcpPort > 0 )
        return listen(QHostAddress::Any, tcpPort, handler);

    this->pPrivate->initialize(ELocalSocket, this);
    this->pPrivate->ihandler = handler;
    return this->pPrivate->ilocalServer->listen(socketOrPort);
}

bool
QHttpServer::listen(const QHostAddress& address, quint16 port, const qhttp::server::TServerHandler& handler) {
    this->pPrivate->initialize(ETcpSocket, this);
    this->pPrivate->ihandler = handler;
    return this->pPrivate->itcpServer->listen(address, port);
}

bool
QHttpServer::isListening() const {
    if ( this->pPrivate->ibackend == ETcpSocket    &&    this->pPrivate->itcpServer )
        return this->pPrivate->itcpServer->isListening();

    else if ( this->pPrivate->ibackend == ELocalSocket    &&    this->pPrivate->ilocalServer )
        return this->pPrivate->ilocalServer->isListening();

    return false;
}

void
QHttpServer::stopListening() {
    if ( this->pPrivate->itcpServer )
        this->pPrivate->itcpServer->close();

    if ( this->pPrivate->ilocalServer ) {
        this->pPrivate->ilocalServer->close();
        QLocalServer::removeServer( this->pPrivate->ilocalServer->fullServerName() );
    }
}

quint32
QHttpServer::timeOut() const {
    return this->pPrivate->itimeOut;
}

void
QHttpServer::setTimeOut(quint32 newValue) {
    this->pPrivate->itimeOut = newValue;
}

TBackend
QHttpServer::backendType() const {
    return this->pPrivate->ibackend;
}

QTcpServer*
QHttpServer::tcpServer() const {
    return this->pPrivate->itcpServer.data();
}

QLocalServer*
QHttpServer::localServer() const {
    return this->pPrivate->ilocalServer.data();
}

void
QHttpServer::incomingConnection(qintptr handle) {
    qDebug()<<"incomming connection2";
    QHttpConnection* conn = new QHttpConnection(this);
    conn->setSocketDescriptor(handle);
    conn->setTimeOut(this->pPrivate->itimeOut);

    emit newConnection(conn);

    if ( this->pPrivate->ihandler )
        QObject::connect(conn, &QHttpConnection::newRequest, this->pPrivate->ihandler);
    else
        incomingConnection(conn);
}

void
QHttpServer::incomingConnection(QHttpConnection *connection) {
    qDebug()<<"incomming connection";
    QObject::connect(connection,  &QHttpConnection::newRequest,
                     this,        &QHttpServer::newRequest);
}

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
