#include "private/qhttpclientresponse_private.hpp"
#include "qhttp/qhttpclient.hpp"
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
QHttpResponse::QHttpResponse(QHttpClient *cli)
    : QHttpAbstractInput(cli), d_ptr(new QHttpResponsePrivate(cli, this)) {
    d_ptr->initialize();
}

QHttpResponse::QHttpResponse(QHttpResponsePrivate &dd, QHttpClient *cli)
    : QHttpAbstractInput(cli), d_ptr(&dd) {
    d_ptr->initialize();
}

QHttpResponse::~QHttpResponse() = default;

TStatusCode
QHttpResponse::status() const {
    return d_func()->istatus;
}

const QString&
QHttpResponse::statusString() const {
    return d_func()->icustomStatusMessage;
}

const QString&
QHttpResponse::httpVersion() const {
    return d_func()->iversion;
}

const Headers&
QHttpResponse::headers() const {
    return d_func()->iheaders;
}

bool
QHttpResponse::isSuccessful() const {
    return d_func()->isuccessful;
}

void
QHttpResponse::collectData(int atMost) {
    d_func()->collectData(atMost);
}

const QByteArray&
QHttpResponse::body() const {
    return d_func()->ibody;
}

QHttpClient*
QHttpResponse::connection() const {
    return d_func()->iclient;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
