#include "private/qhttpclient_private.hpp"

#include <QTimerEvent>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
QHttpClient::QHttpClient(QObject *parent)
    : QTcpSocket(parent), d_ptr(new QHttpClientPrivate(this)) {
    d_ptr->initialize();
    QHTTP_LINE_LOG
}

QHttpClient::QHttpClient(QHttpClientPrivate &dd, QObject *parent)
    : QTcpSocket(parent), d_ptr(&dd) {
    d_ptr->initialize();
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
    return QTcpSocket::isOpen()    &&    state() == QTcpSocket::ConnectedState;
}

bool
QHttpClient::request(THttpMethod method, QUrl url,
                     const TRequstHandler &reqHandler, const TResponseHandler &resHandler) {
    Q_D(QHttpClient);

    d->ireqHandler   = nullptr;
    d->irespHandler  = nullptr;

    if ( !url.isValid()    ||    url.isEmpty()    ||    url.host().isEmpty() )
        return false;

    d->ilastMethod  = method;
    d->ilastUrl     = url;

    connectToHost(url.host(), url.port(80));

    // process handlers
    if ( resHandler ) {
        d->irespHandler = resHandler;

        if ( reqHandler )
            d->ireqHandler = reqHandler;
        else
            d->ireqHandler = [](QHttpRequest* req) ->void { req->end(); };
    }

    return true;

}

void
QHttpClient::timerEvent(QTimerEvent *e) {
    Q_D(QHttpClient);

    if ( e->timerId() == d->itimer.timerId() ) {
        close();
    }
}

void
QHttpClient::onRequestReady(QHttpRequest *req) {
    emit httpConnected(req);
}

void
QHttpClient::onResponseReady(QHttpResponse *res) {
    emit newResponse(res);
}

///////////////////////////////////////////////////////////////////////////////

void
QHttpClientPrivate::onConnected() {
    QHttpRequest *request = new QHttpRequest(isocket);

    request->d_func()->imethod  = ilastMethod;
    request->d_func()->iurl     = ilastUrl;

    if ( itimeOut > 0 )
        itimer.start(itimeOut, Qt::CoarseTimer, q_func());

#   if QHTTP_MESSAGES_LOG > 0
    iinputBuffer.clear();
#   endif

    if ( ireqHandler )
        ireqHandler(request);
    else
        q_func()->onRequestReady(request);
}

void
QHttpClientPrivate::onReadyRead() {
    while ( isocket->bytesAvailable() > 0 ) {
        char buffer[4097] = {0};
        size_t readLength = isocket->read(buffer, 4096);

        parse(buffer, readLength);

#       if QHTTP_MESSAGES_LOG > 0
        iinputBuffer.append(buffer);
#       endif
    }
}

///////////////////////////////////////////////////////////////////////////////

// if user closes the connection, ends the response or by any other reason
//  the socket be disconnected, then the iresponse instance may has been deleted.
//  In these situations reading more http body or emitting end() for incoming response
//  is not possible.
#define CHECK_FOR_DISCONNECTED  if ( ilastResponse == nullptr ) \
    return 0;


int
QHttpClientPrivate::messageBegin(http_parser*) {
    itempHeaderField.clear();
    itempHeaderValue.clear();

    return 0;
}

int
QHttpClientPrivate::status(http_parser* parser, const char* at, size_t length) {
    CHECK_FOR_DISCONNECTED

    ilastResponse = new QHttpResponse(isocket);
    ilastResponse->d_func()->istatus  = static_cast<TStatusCode>(parser->status_code);
    ilastResponse->d_func()->iversion = QString("%1.%2")
                                        .arg(parser->http_major)
                                        .arg(parser->http_minor);
    ilastResponse->d_func()->icustomeStatusMessage = QString::fromUtf8(at, length);

    return 0;
}

int
QHttpClientPrivate::headerField(http_parser*, const char* at, size_t length) {
    CHECK_FOR_DISCONNECTED

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

    itempHeaderValue.append(at, length);
    return 0;
}

int
QHttpClientPrivate::headersComplete(http_parser*) {
    CHECK_FOR_DISCONNECTED

    // Insert last remaining header
    ilastResponse->d_func()->iheaders.insert(
                itempHeaderField.toLower(),
                itempHeaderValue.toLower()
                );

    if ( irespHandler )
        irespHandler(ilastResponse);
    else
        q_func()->onResponseReady(ilastResponse);

    return 0;
}

int
QHttpClientPrivate::body(http_parser*, const char* at, size_t length) {
    CHECK_FOR_DISCONNECTED

    if ( ilastResponse->idataHandler )
        ilastResponse->idataHandler(QByteArray(at, length));
    else
        emit ilastResponse->data(QByteArray(at, length));

    return 0;
}

int
QHttpClientPrivate::messageComplete(http_parser*) {
    CHECK_FOR_DISCONNECTED

    ilastResponse->d_func()->isuccessful = true;

    if ( ilastResponse->iendHandler )
        ilastResponse->iendHandler();
    else
        emit ilastResponse->end();

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
