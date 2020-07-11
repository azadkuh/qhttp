#include "private/qhttpclientresponse_private.hpp"
#include "qhttpclient.hpp"
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
QHttpResponse::QHttpResponse(QHttpClient *cli)
    : QHttpAbstractInput(cli), pPrivate(new QHttpResponsePrivate(cli, this)) {
    pPrivate->initialize();
    QHTTP_LINE_LOG
}

QHttpResponse::QHttpResponse(QHttpResponsePrivate &dd, QHttpClient *cli)
    : QHttpAbstractInput(cli), pPrivate(&dd) {
    pPrivate->initialize();
    QHTTP_LINE_LOG
}

QHttpResponse::~QHttpResponse() {
    QHTTP_LINE_LOG
}

TStatusCode
QHttpResponse::status() const {
    return this->pPrivate->istatus;
}

const QString&
QHttpResponse::statusString() const {
    return this->pPrivate->icustomStatusMessage;
}

const QString&
QHttpResponse::httpVersion() const {
    return this->pPrivate->iversion;
}

const THeaderHash&
QHttpResponse::headers() const {
    return this->pPrivate->iheaders;
}

bool
QHttpResponse::isSuccessful() const {
    return this->pPrivate->isuccessful;
}

void
QHttpResponse::collectData(int atMost) {
    this->pPrivate->collectData(atMost);
}

const QByteArray&
QHttpResponse::collectedData() const {
    return this->pPrivate->icollectedData;
}

QHttpClient*
QHttpResponse::connection() const {
    return this->pPrivate->iclient;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
