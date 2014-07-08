#include "private/qhttpclient_private.hpp"

#include <QTimerEvent>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
QHttpClient::QHttpClient(QObject *parent)
    : QObject(parent), d_ptr(new QHttpClientPrivate(this)) {
    QHTTP_LINE_LOG
}

QHttpClient::QHttpClient(QHttpClientPrivate &dd, QObject *parent)
    : QObject(parent), d_ptr(&dd) {
    QHTTP_LINE_LOG
}

QHttpClient::~QHttpClient() {
    QHTTP_LINE_LOG
}

quint32
QHttpClient::timeOut() const {
    return d_func()->itimeOut;
}

void
QHttpClient::setTimeOut(quint32 t) {
    d_func()->itimeOut = t;
}

bool
QHttpClient::isOpen() const {
    return d_func()->isocket->isOpen();
}

void
QHttpClient::close() {
    d_func()->isocket->disconnectFromHost();
    d_func()->isocket->close();
}

void
QHttpClient::request(THttpMethod method, QUrl url) {
    Q_D(QHttpClient);

    d->ilastMethod  = method;
    d->ilastUrl     = url;

    d->isocket->connectToHost(url.host(), url.port(80));
}

void
QHttpClient::timerEvent(QTimerEvent *e) {
    Q_D(QHttpClient);

    if ( e->timerId() == d->itimer.timerId() ) {
        d->isocket->disconnectFromHost();
    }
}

void
QHttpClient::onRequestReady(QHttpRequest *req) {
    emit connected(req);
}

void
QHttpClient::onResponseReady(QHttpResponse *res) {
    emit newResponse(res);
}

///////////////////////////////////////////////////////////////////////////////

int
QHttpClientPrivate::messageBegin(http_parser*) {
    itempHeaderField.clear();
    itempHeaderValue.clear();

    ilastResponse = new QHttpResponse(isocket);
    return 0;
}

int
QHttpClientPrivate::status(http_parser* parser, const char* at, size_t length) {
    Q_ASSERT(ilastResponse);

    ilastResponse->d_func()->istatus  = static_cast<TStatusCode>(parser->status_code);
    ilastResponse->d_func()->iversion = QString("%1.%2")
                                        .arg(parser->http_major)
                                        .arg(parser->http_minor);
    ilastResponse->d_func()->icustomeStatusMessage = QString::fromUtf8(at, length);

    return 0;
}

int
QHttpClientPrivate::headerField(http_parser*, const char* at, size_t length) {
    Q_ASSERT(ilastResponse);

    // insert the header we parsed previously
    // into the header map
    if ( !itempHeaderField.isEmpty() && !itempHeaderValue.isEmpty() ) {
        // header names are always lower-cased
        ilastResponse->d_func()->iheaders.insert(
                    itempHeaderField.toLower(),
                    itempHeaderValue.toLower()
                    );
        // clear header value. this sets up a nice
        // feedback loop where the next time
        // HeaderValue is called, it can simply append
        itempHeaderField.clear();
        itempHeaderValue.clear();
    }

    itempHeaderField.append(at, length);
    return 0;
}

int
QHttpClientPrivate::headerValue(http_parser*, const char* at, size_t length) {
    Q_ASSERT(ilastResponse);

    itempHeaderValue.append(at, length);
    return 0;
}

int
QHttpClientPrivate::headersComplete(http_parser*) {
    Q_ASSERT(ilastResponse);

    // Insert last remaining header
    ilastResponse->d_func()->iheaders.insert(
                itempHeaderField.toLower(),
                itempHeaderValue.toLower()
                );

    emit q_func()->onResponseReady(ilastResponse);
    return 0;
}

int
QHttpClientPrivate::body(http_parser*, const char* at, size_t length) {
    Q_ASSERT(ilastResponse);

    emit ilastResponse->data(QByteArray(at, length));
    return 0;
}

int
QHttpClientPrivate::messageComplete(http_parser*) {
    Q_ASSERT(ilastResponse);

    ilastResponse->d_func()->isuccessful = true;
    emit ilastResponse->end();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
