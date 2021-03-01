#include "private/qhttpclient_private.hpp"

#include <QTimerEvent>
///////////////////////////////////////////////////////////////////////////////
#ifdef QHTTP_HAS_CLIENT
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
QHttpClient::QHttpClient(QObject* parent, qhttp::TBackend backendType)
    : QObject(parent), pPrivate(new QHttpClientPrivate(this, backendType)) {
    QHTTP_LINE_LOG
}

QHttpClient::QHttpClient(QHttpClientPrivate &dd, QObject *parent)
    : QObject(parent), pPrivate(&dd) {
    QHTTP_LINE_LOG
}

QHttpClient::~QHttpClient() {
    QHTTP_LINE_LOG
}

quint32
QHttpClient::timeOut() const {
    return this->pPrivate->itimeOut;
}

void
QHttpClient::setTimeOut(quint32 t) {
    this->pPrivate->itimeOut = t;
}

bool
QHttpClient::isOpen() const {
    return this->pPrivate->isocket->isOpen();
}

void
QHttpClient::killConnection() {
    this->pPrivate->iconnectingTimer.stop();
    this->pPrivate->itimer.stop();
    this->pPrivate->isocket->close();
}

TBackend
QHttpClient::backendType() const {
    return (dynamic_cast<details::QHttpTcpSocket*>(this->pPrivate->isocket->isocket) != nullptr) ?
        ETcpSocket :
        ELocalSocket;
}

QTcpSocket*
QHttpClient::tcpSocket() const {
    return backendType() == ETcpSocket? dynamic_cast<QTcpSocket*>(this->pPrivate->isocket->isocket) : nullptr;
}

QLocalSocket*
QHttpClient::localSocket() const {
    return backendType() == ELocalSocket ? dynamic_cast<QLocalSocket*>(this->pPrivate->isocket->isocket) : nullptr;
}

details::QHttpAbstractSocket*
QHttpClient::abstractSocket() const
{
    return this->pPrivate->isocket.data();
}

void
QHttpClient::setConnectingTimeOut(quint32 timeout) {
    if ( timeout == 0 ) {
        this->pPrivate->iconnectingTimer.stop();

    } else {
        this->pPrivate->iconnectingTimer.start(static_cast<int>(timeout),
                Qt::CoarseTimer,
                this
                );
    }
}

bool
QHttpClient::request(THttpMethod method, QUrl url,
                     const TRequstHandler &reqHandler,
                     const TResponseHandler &resHandler) {
    this->pPrivate->ireqHandler   = nullptr;
    this->pPrivate->irespHandler  = nullptr;

    // if url is a local file (UNIX socket) the host could be empty!
    if ( !url.isValid()    ||    url.isEmpty()    /*||    url.host().isEmpty()*/ )
        return false;

    // process handlers
    if ( resHandler ) {
        this->pPrivate->irespHandler = resHandler;

        if ( reqHandler )
            this->pPrivate->ireqHandler = reqHandler;
        else
            this->pPrivate->ireqHandler = [](QHttpRequest* req) ->void {
                req->addHeader("connection", "close");
                req->end();
            };
    }

    auto requestCreator = [this, method, url]() {
        // create request object
        if (this->pPrivate->ilastRequest )
            this->pPrivate->ilastRequest->deleteLater();

        this->pPrivate->ilastRequest = new QHttpRequest(this);
        QObject::connect(this->pPrivate->ilastRequest, &QHttpRequest::done, [this](bool wasTheLastPacket){
            this->pPrivate->ikeepAlive = !wasTheLastPacket;
        });

        this->pPrivate->ilastRequest->pPrivate->imethod  = method;
        this->pPrivate->ilastRequest->pPrivate->iurl     = url;
    };

    // connecting to host/server must be the last thing. (after all function handlers and ...)
    // check for type
    if ( url.scheme().toLower() == QLatin1String("file") ) {
        //d->isocket->ibackendType = ELocalSocket;
        this->pPrivate->initializeSocket();

        requestCreator();

        if ( this->pPrivate->isocket->isOpen() )
            this->pPrivate->onConnected();
        else
            this->pPrivate->isocket->connectTo(url.path());

    } else {
        //d->isocket->ibackendType = ETcpSocket;
        this->pPrivate->initializeSocket();

        requestCreator();

        if ( this->pPrivate->isocket->isOpen() )
            this->pPrivate->onConnected();
        else
            this->pPrivate->isocket->connectTo(url.host(), static_cast<quint16>(url.port(80)));
    }

    return true;
}

void
QHttpClient::timerEvent(QTimerEvent *e) {
    if ( e->timerId() == this->pPrivate->itimer.timerId() ) {
        killConnection();

    } else if ( e->timerId() == this->pPrivate->iconnectingTimer.timerId() ) {
        this->pPrivate->iconnectingTimer.stop();
        emit connectingTimeOut();
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

// if server closes the connection, ends the response or by any other reason
// the socket disconnects, then the irequest and iresponse instances may have
// been deleted. In these situations reading more http body or emitting end()
// for incoming request are not possible:
// if ( ilastRequest == nullptr )
//     return 0;

int
QHttpClientPrivate::messageBegin(http_parser*) {
    itempHeaderField.clear();
    itempHeaderValue.clear();

    return 0;
}

int
QHttpClientPrivate::status(http_parser* parser, const char* at, int length) {
    if ( ilastResponse )
        ilastResponse->deleteLater();

    ilastResponse = new QHttpResponse(q_func());
    ilastResponse->pPrivate->istatus  = static_cast<TStatusCode>(parser->status_code);
    ilastResponse->pPrivate->iversion = QString("%1.%2")
                                        .arg(parser->http_major)
                                        .arg(parser->http_minor);
    ilastResponse->pPrivate->icustomStatusMessage = QString::fromUtf8(at, length);

    return 0;
}

int
QHttpClientPrivate::headerField(http_parser*, const char* at, int length) {
    if ( ilastResponse == nullptr )
        return 0;

    // insert the header we parsed previously
    // into the header map
    if ( !itempHeaderField.isEmpty() && !itempHeaderValue.isEmpty() ) {
        // header names are always lower-cased
        ilastResponse->pPrivate->iheaders.insert(
                    itempHeaderField.toLower(),
                    itempHeaderValue
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
QHttpClientPrivate::headerValue(http_parser*, const char* at, int length) {

    itempHeaderValue.append(at, length);
    return 0;
}

int
QHttpClientPrivate::headersComplete(http_parser*) {
    if ( ilastResponse == nullptr )
        return 0;

    // Insert last remaining header
    ilastResponse->pPrivate->iheaders.insert(
                itempHeaderField.toLower(),
                itempHeaderValue
                );

    if ( irespHandler )
        irespHandler(ilastResponse);
    else
        q_func()->onResponseReady(ilastResponse);

    return 0;
}

int
QHttpClientPrivate::body(http_parser*, const char* at, int length) {
    if ( ilastResponse == nullptr )
        return 0;

    ilastResponse->pPrivate->ireadState = QHttpResponsePrivate::EPartial;

    if ( ilastResponse->pPrivate->icollectRequired ) {
        if ( !ilastResponse->pPrivate->append(at, length) ) {
            // forcefully dispatch the ilastResponse
            finalizeConnection();
        }

        return 0;
    }

    emit ilastResponse->data(QByteArray(at, length));
    return 0;
}

int
QHttpClientPrivate::messageComplete(http_parser*) {
    if ( ilastResponse == nullptr )
        return 0;

    // response is done
    finalizeConnection();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif //QHTTP_HAS_CLIENT
