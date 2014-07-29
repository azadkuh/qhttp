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
    const Q_D(QHttpClient);

    if ( d->ibackendType == ETcpSocket    &&    d->itcpSocket ) {
        return d->itcpSocket->isOpen()    &&    d->itcpSocket->state() == QTcpSocket::ConnectedState;

    } else if ( d->ibackendType == ELocalSocket    &&    d->ilocalSocket ) {
        return d->ilocalSocket->isOpen()    &&    d->ilocalSocket->state() == QLocalSocket::ConnectedState;
    }

    return false;
}

void
QHttpClient::killConnection() {
    Q_D(QHttpClient);

    if ( d->itcpSocket )
        d->itcpSocket->disconnectFromHost();
    if ( d->ilocalSocket )
        d->ilocalSocket->disconnectFromServer();
}

TBackend
QHttpClient::backendType() const {
    return d_func()->ibackendType;
}

QTcpSocket*
QHttpClient::tcpSocket() const {
    return d_func()->itcpSocket;
}

QLocalSocket*
QHttpClient::localSocket() const {
    return d_func()->ilocalSocket;
}

bool
QHttpClient::request(THttpMethod method, QUrl url,
                     const TRequstHandler &reqHandler, const TResponseHandler &resHandler) {
    Q_D(QHttpClient);

    d->ireqHandler   = nullptr;
    d->irespHandler  = nullptr;

    // if url is a local file (UNIX socket) the host could be empty!
    if ( !url.isValid()    ||    url.isEmpty()    /*||    url.host().isEmpty()*/ )
        return false;

    d->ilastMethod  = method;
    d->ilastUrl     = url;

    // process handlers
    if ( resHandler ) {
        d->irespHandler = resHandler;

        if ( reqHandler )
            d->ireqHandler = reqHandler;
        else
            d->ireqHandler = [](QHttpRequest* req) ->void { req->end(); };
    }

    // connecting to host/server must be the last thing. (after all function handlers and ...)
    // check for type
    if ( url.scheme().toLower() == QLatin1String("file") ) {
        d->ibackendType = ELocalSocket;
        d->initializeSocket();

        d->ilocalSocket->connectToServer(url.path());

    } else {
        d->ibackendType = ETcpSocket;
        d->initializeSocket();

        d->itcpSocket->connectToHost(url.host(), url.port(80));
    }

    return true;
}

void
QHttpClient::timerEvent(QTimerEvent *e) {
    Q_D(QHttpClient);

    if ( e->timerId() == d->itimer.timerId() ) {
        killConnection();
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
    QHttpRequest *request = new QHttpRequest(q_func());

    request->d_func()->imethod  = ilastMethod;
    request->d_func()->iurl     = ilastUrl;

    if ( itimeOut > 0 )
        itimer.start(itimeOut, Qt::CoarseTimer, q_func());

    if ( ireqHandler )
        ireqHandler(request);
    else
        q_func()->onRequestReady(request);
}

void
QHttpClientPrivate::onReadyRead() {
    if ( itcpSocket ) {
        while ( itcpSocket->bytesAvailable() > 0 ) {
            char buffer[4097] = {0};
            size_t readLength = itcpSocket->read(buffer, 4096);

            parse(buffer, readLength);
        }

    } else if ( ilocalSocket ) {
        while ( ilocalSocket->bytesAvailable() > 0 ) {
            char buffer[4097] = {0};
            size_t readLength = ilocalSocket->read(buffer, 4096);

            parse(buffer, readLength);
        }
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

    ilastResponse = new QHttpResponse(q_func());
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

    // prevents double messageComplete!
    if ( ilastResponse->d_func()->isuccessful )
        return 0;

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
