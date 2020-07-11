#include "private/qhttpserverconnection_private.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
QHttpConnection::QHttpConnection(QObject *parent, TBackend backendType)
    : QObject(parent), pPrivate(new QHttpConnectionPrivate(this, backendType)) {
    QHTTP_LINE_LOG
}

QHttpConnection::QHttpConnection(QHttpConnectionPrivate& dd, QObject* parent)
    : QObject(parent), pPrivate(&dd) {
    QHTTP_LINE_LOG
}

void
QHttpConnection::setSocketDescriptor(qintptr sokDescriptor) {
    this->pPrivate->createSocket(sokDescriptor);
}

QHttpConnection::~QHttpConnection() {
    QHTTP_LINE_LOG
}

void
QHttpConnection::setTimeOut(quint32 miliSeconds) {
    if ( miliSeconds != 0 ) {
        this->pPrivate->itimeOut = miliSeconds;
        this->pPrivate->itimer.start(static_cast<int>(miliSeconds), Qt::CoarseTimer, this);
    }
}

void
QHttpConnection::killConnection() {
    this->pPrivate->isocket->close();
}

TBackend
QHttpConnection::backendType() const {
    return (dynamic_cast<QTcpSocket*>(this->pPrivate->isocket->isocket) != nullptr) ?
        ETcpSocket :
        ELocalSocket;
}

QTcpSocket*
QHttpConnection::tcpSocket() const {
    return backendType() == ETcpSocket? dynamic_cast<QTcpSocket*>(this->pPrivate->isocket->isocket) : nullptr;
}

QLocalSocket*
QHttpConnection::localSocket() const {
    return backendType() == ELocalSocket ? dynamic_cast<QLocalSocket*>(this->pPrivate->isocket->isocket) : nullptr;
}

details::QHttpAbstractSocket*
QHttpConnection::abstractSocket() const {
    return this->pPrivate->isocket.data();
}


void
QHttpConnection::onHandler(const TServerHandler &handler) {
    this->pPrivate->ihandler = handler;
}

void
QHttpConnection::timerEvent(QTimerEvent *) {
    killConnection();
}

///////////////////////////////////////////////////////////////////////////////

// if user closes the connection, ends the response or by any other reason the
// socket disconnects, then the irequest and iresponse instances may have
// been deleted. In these situations reading more http body or emitting end()
// for incoming request are not possible:
// if ( ilastRequest == nullptr )
//     return 0;


int
QHttpConnectionPrivate::messageBegin(http_parser*) {
    itempUrl.clear();
    itempUrl.reserve(128);

    if ( ilastRequest )
        ilastRequest->deleteLater();

    ilastRequest = new QHttpRequest(q_func());
    return 0;
}

int
QHttpConnectionPrivate::url(http_parser*, const char* at, int length) {
    Q_ASSERT(ilastRequest);

    itempUrl.append(at, length);
    return 0;
}

int
QHttpConnectionPrivate::headerField(http_parser*, const char* at, int length) {
    if ( ilastRequest == nullptr )
        return 0;

    // insert the header we parsed previously
    // into the header map
    if ( !itempHeaderField.isEmpty() && !itempHeaderValue.isEmpty() ) {
        // header names are always lower-cased
        ilastRequest->pPrivate->iheaders.insert(
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
QHttpConnectionPrivate::headerValue(http_parser*, const char* at, int length) {
    if ( ilastRequest == nullptr )
        return 0;

    itempHeaderValue.append(at, length);
    return 0;
}

int
QHttpConnectionPrivate::headersComplete(http_parser* parser) {
    if ( ilastRequest == nullptr )
        return 0;

    ilastRequest->pPrivate->iurl = QUrl(itempUrl);

    // set method
    ilastRequest->pPrivate->imethod =
            static_cast<THttpMethod>(parser->method);

    // set version
    ilastRequest->pPrivate->iversion = QString("%1.%2")
                                       .arg(parser->http_major)
                                       .arg(parser->http_minor);

    // Insert last remaining header
    ilastRequest->pPrivate->iheaders.insert(
                itempHeaderField.toLower(),
                itempHeaderValue
                );

    // set client information
    ilastRequest->pPrivate->iremoteAddress = isocket->remoteAddress();
    ilastRequest->pPrivate->iremotePort = isocket->remotePort();

    if ( ilastResponse )
        ilastResponse->deleteLater();
    ilastResponse  = new QHttpResponse(q_func());

    if ( parser->http_major < 1 || parser->http_minor < 1  )
        ilastResponse->pPrivate->ikeepAlive = false;

    // close the connection if response was the last packet
    QObject::connect(ilastResponse, &QHttpResponse::done, [this](bool wasTheLastPacket){
        ikeepAlive = !wasTheLastPacket;
        if ( wasTheLastPacket ) {
            isocket->flush();
            isocket->close();
        }
    });

    // we are good to go!
    if ( ihandler )
        ihandler(ilastRequest, ilastResponse);
    else
        emit q_ptr->newRequest(ilastRequest, ilastResponse);

    return 0;
}

int
QHttpConnectionPrivate::body(http_parser*, const char* at, int length) {
    if ( ilastRequest == nullptr )
        return 0;

    ilastRequest->pPrivate->ireadState = QHttpRequestPrivate::EPartial;

    if ( ilastRequest->pPrivate->icollectRequired ) {
        if ( !ilastRequest->pPrivate->append(at, length) ) {
            // forcefully dispatch the ilastRequest
            finalizeConnection();
        }

        return 0;
    }

    emit ilastRequest->data(QByteArray(at, length));
    return 0;
}

int
QHttpConnectionPrivate::messageComplete(http_parser*) {
    if ( ilastRequest == nullptr )
        return 0;

    // request is done
    finalizeConnection();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
