#include "private/qhttpserver_private.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////

QHttpServer::QHttpServer(QObject *parent)
    : QTcpServer(parent), d_ptr(new QHttpServerPrivate) {
}

QHttpServer::QHttpServer(QHttpServerPrivate &dd, QObject *parent)
    : QTcpServer(parent), d_ptr(&dd) {
}

QHttpServer::~QHttpServer() {
}

bool
QHttpServer::listen(const QHostAddress& address, quint16 port) {
    return QTcpServer::listen(address, port);
}

quint32
QHttpServer::timeOut() const {
    return d_func()->itimeOut;
}

void
QHttpServer::setTimeOut(quint32 newValue) {
    d_func()->itimeOut = newValue;
}

void
QHttpServer::incomingConnection(qintptr handle) {
    QHttpConnection* conn = new QHttpConnection(this);
    conn->setSocketDescriptor(handle);
    conn->setTimeOut(d_func()->itimeOut);

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
