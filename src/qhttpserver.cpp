#include "private/qhttpserver_private.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////

QHttpServer::QHttpServer(QObject *parent)
    : QObject(parent), d_ptr(new QHttpServerPrivate) {
}

QHttpServer::QHttpServer(QHttpServerPrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd) {
}

QHttpServer::~QHttpServer() {
}

bool
QHttpServer::listen(const QString &socket, const TServerHandler &handler) {
    Q_D(QHttpServer);

    d->initialize(ELocalSocket, this);
    d->ihandler = handler;
    return d->ilocalServer->listen(socket);
}

bool
QHttpServer::listen(const QHostAddress& address, quint16 port, const qhttp::server::TServerHandler& handler) {
    Q_D(QHttpServer);

    d->initialize(ETcpSocket, this);
    d->ihandler = handler;
    return d->itcpServer->listen(address, port);
}

bool
QHttpServer::isListening() const {
    const Q_D(QHttpServer);

    if ( d->ibackend == ETcpSocket    &&    d->itcpServer )
        return d->itcpServer->isListening();

    else if ( d->ibackend == ELocalSocket    &&    d->ilocalServer )
        return d->ilocalServer->isListening();

    return false;
}

quint32
QHttpServer::timeOut() const {
    return d_func()->itimeOut;
}

void
QHttpServer::setTimeOut(quint32 newValue) {
    d_func()->itimeOut = newValue;
}

TBackend
QHttpServer::backendType() const {
    return d_func()->ibackend;
}

QTcpServer*
QHttpServer::tcpServer() const {
    return d_func()->itcpServer.data();
}

QLocalServer*
QHttpServer::localServer() const {
    return d_func()->ilocalServer.data();
}

void
QHttpServer::incomingConnection(qintptr handle) {
    QHttpConnection* conn = new QHttpConnection(this);
    conn->setSocketDescriptor(handle, backendType());
    conn->setTimeOut(d_func()->itimeOut);

    Q_D(QHttpServer);
    if ( d->ihandler )
        QObject::connect(conn, &QHttpConnection::newRequest, d->ihandler);
    else
        incomingConnection(conn);
}

void
QHttpServer::incomingConnection(QHttpConnection *connection) {
    QObject::connect(connection,  &QHttpConnection::newRequest,
                     this,        &QHttpServer::newRequest);
}

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
