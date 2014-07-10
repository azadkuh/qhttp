#include "private/qhttpclientresponse_private.hpp"
#include "qhttpclient.hpp"
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
QHttpResponse::QHttpResponse(QTcpSocket *socket)
    : QHttpAbstractInput(socket), d_ptr(new QHttpResponsePrivate(socket, this)) {
    d_ptr->initialize();
    QHTTP_LINE_LOG
}

QHttpResponse::QHttpResponse(QHttpResponsePrivate &dd, QTcpSocket *socket)
    : QHttpAbstractInput(socket), d_ptr(&dd) {
    d_ptr->initialize();
    QHTTP_LINE_LOG
}

QHttpResponse::~QHttpResponse() {
    QHTTP_LINE_LOG
}

TStatusCode
QHttpResponse::status() const {
    return d_func()->istatus;
}

const QString&
QHttpResponse::statusString() const {
    return d_func()->icustomeStatusMessage;
}

const QString&
QHttpResponse::httpVersion() const {
    return d_func()->iversion;
}

const THeaderHash&
QHttpResponse::headers() const {
    return d_func()->iheaders;
}

bool
QHttpResponse::isSuccessful() const {
    return d_func()->isuccessful;
}

QHttpClient*
QHttpResponse::connection() const {
    return static_cast<QHttpClient* const>(d_func()->isocket);
}

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
