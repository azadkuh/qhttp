#include "private/qhttpserverrequest_private.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
QHttpRequest::QHttpRequest(QHttpConnection *conn)
    : QHttpAbstractInput(conn), pPrivate(new QHttpRequestPrivate(conn, this)) {
    this->pPrivate->initialize();
    QHTTP_LINE_LOG
}

QHttpRequest::QHttpRequest(QHttpRequestPrivate &dd, QHttpConnection *conn)
    : QHttpAbstractInput(conn), pPrivate(&dd) {
    this->pPrivate->initialize();
    QHTTP_LINE_LOG
}

QHttpRequest::~QHttpRequest() {
    QHTTP_LINE_LOG
}

THttpMethod
QHttpRequest::method() const {
    return this->pPrivate->imethod;
}

const QString
QHttpRequest::methodString() const {
    return http_method_str(static_cast<http_method>(this->pPrivate->imethod));
}

const QUrl&
QHttpRequest::url() const {
    return this->pPrivate->iurl;
}

const QString&
QHttpRequest::httpVersion() const {
    return this->pPrivate->iversion;
}

const THeaderHash&
QHttpRequest::headers() const {
    return this->pPrivate->iheaders;
}

const QString&
QHttpRequest::remoteAddress() const {
    return this->pPrivate->iremoteAddress;
}

quint16
QHttpRequest::remotePort() const {
    return this->pPrivate->iremotePort;
}

bool
QHttpRequest::isSuccessful() const {
    return this->pPrivate->isuccessful;
}

void
QHttpRequest::collectData(int atMost) {
    this->pPrivate->collectData(atMost);
}

const QByteArray&
QHttpRequest::collectedData() const {
    return this->pPrivate->icollectedData;
}

QHttpConnection*
QHttpRequest::connection() const {
    return pPrivate ? this->pPrivate->iconnection : nullptr;
}

void QHttpRequest::addUserDefinedData(const QString &key, const QString value) {
    if(pPrivate) this->pPrivate->iuserDefinedValues.append(qMakePair(key, value));
}

QList<QPair<QString, QString>> QHttpRequest::userDefinedValues() const{
  return pPrivate ? this->pPrivate->iuserDefinedValues : QList<QPair<QString, QString>>();
}

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
