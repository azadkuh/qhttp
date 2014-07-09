#include "private/qhttpserverrequest_private.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
QHttpRequest::QHttpRequest(QTcpSocket *socket)
    : QHttpAbstractInput(socket), d_ptr(new QHttpRequestPrivate(socket, this)) {
    d_ptr->initialize();
    QHTTP_LINE_LOG
}

QHttpRequest::QHttpRequest(QHttpRequestPrivate &dd, QTcpSocket *socket)
    : QHttpAbstractInput(socket), d_ptr(&dd) {
    d_ptr->initialize();
    QHTTP_LINE_LOG
}

QHttpRequest::~QHttpRequest() {
    QHTTP_LINE_LOG
}

THttpMethod
QHttpRequest::method() const {
    return d_func()->imethod;
}

const QString
QHttpRequest::methodString() const {
    return http_method_str(static_cast<http_method>(d_func()->imethod));
}

const QUrl&
QHttpRequest::url() const {
    return d_func()->iurl;
}

const QString&
QHttpRequest::httpVersion() const {
    return d_func()->iversion;
}

const THeaderHash&
QHttpRequest::headers() const {
    return d_func()->iheaders;
}

const QString&
QHttpRequest::remoteAddress() const {
    return d_func()->iremoteAddress;
}

quint16
QHttpRequest::remotePort() const {
    return d_func()->iremotePort;
}

bool
QHttpRequest::isSuccessful() const {
    return d_func()->isuccessful;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
