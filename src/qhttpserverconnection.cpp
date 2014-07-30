#include "private/qhttpserverconnection_private.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
QHttpConnection::QHttpConnection(QObject *parent)
    : QObject(parent), d_ptr(new QHttpConnectionPrivate(this)) {
    QHTTP_LINE_LOG
}

QHttpConnection::QHttpConnection(QHttpConnectionPrivate& dd, QObject* parent)
    : QObject(parent), d_ptr(&dd) {
    QHTTP_LINE_LOG
}

void
QHttpConnection::setSocketDescriptor(qintptr sokDescriptor, TBackend backendType) {
    d_ptr->initialize(sokDescriptor, backendType);
}

QHttpConnection::~QHttpConnection() {
    QHTTP_LINE_LOG
}

void
QHttpConnection::setTimeOut(quint32 miliSeconds) {
    if ( miliSeconds != 0 )
        d_func()->itimer.start(miliSeconds, Qt::CoarseTimer, this);
}

void
QHttpConnection::killConnection() {
    Q_D(QHttpConnection);

    if ( d->ibackendType == ETcpSocket )
        d->itcpSocket->disconnectFromHost();
    else if ( d->ibackendType == ELocalSocket )
        d->ilocalSocket->disconnectFromServer();
}

TBackend
QHttpConnection::backendType() const {
    return d_func()->ibackendType;
}

QTcpSocket*
QHttpConnection::tcpSocket() const {
    return d_func()->itcpSocket;
}

QLocalSocket*
QHttpConnection::localSocket() const {
    return d_func()->ilocalSocket;
}

void
QHttpConnection::onHandler(const TServerHandler &handler) {
    d_func()->ihandler = handler;
}

void
QHttpConnection::timerEvent(QTimerEvent *) {
    killConnection();
}

///////////////////////////////////////////////////////////////////////////////

// if user closes the connection, ends the response or by any other reason
//  the socket be disconnected, then the irequest and iresponse instances may bhave been deleted.
//  In these situations reading more http body or emitting end() for incoming request
//  are not possible.
#define CHECK_FOR_DISCONNECTED  if ( irequest == nullptr ) \
    return 0;


int
QHttpConnectionPrivate::messageBegin(http_parser*) {
    itempUrl.clear();
    itempUrl.reserve(128);

    Q_ASSERT( irequest == nullptr );
    irequest = new QHttpRequest(q_func());

    return 0;
}

int
QHttpConnectionPrivate::url(http_parser*, const char* at, size_t length) {
    Q_ASSERT(irequest);

    itempUrl.append(at, length);
    return 0;
}

int
QHttpConnectionPrivate::headerField(http_parser*, const char* at, size_t length) {
    CHECK_FOR_DISCONNECTED

    // insert the header we parsed previously
    // into the header map
    if ( !itempHeaderField.isEmpty() && !itempHeaderValue.isEmpty() ) {
        // header names are always lower-cased
        irequest->d_func()->iheaders.insert(
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
QHttpConnectionPrivate::headerValue(http_parser*, const char* at, size_t length) {
    CHECK_FOR_DISCONNECTED

    itempHeaderValue.append(at, length);
    return 0;
}

int
QHttpConnectionPrivate::headersComplete(http_parser* parser) {
    CHECK_FOR_DISCONNECTED

#if defined(USE_CUSTOM_URL_CREATOR)
    // get parsed url
    struct http_parser_url urlInfo;
    int r = http_parser_parse_url(itempUrl.constData(),
                                  itempUrl.size(),
                                  parser->method == HTTP_CONNECT,
                                  &urlInfo);
    Q_ASSERT(r == 0);
    Q_UNUSED(r);

    irequest->d_func()->iurl = createUrl(
                                 itempUrl.constData(),
                                 urlInfo
                                 );
#else
    irequest->d_func()->iurl = QUrl(itempUrl);
#endif // defined(USE_CUSTOM_URL_CREATOR)

    // set method
    irequest->d_func()->imethod =
            static_cast<THttpMethod>(parser->method);

    // set version
    irequest->d_func()->iversion = QString("%1.%2")
                                 .arg(parser->http_major)
                                 .arg(parser->http_minor);

    // Insert last remaining header
    irequest->d_func()->iheaders.insert(
                itempHeaderField.toLower(),
                itempHeaderValue.toLower()
                );

    // set client information
    if ( ibackendType == ETcpSocket ) {
        irequest->d_func()->iremoteAddress = itcpSocket->peerAddress().toString();
        irequest->d_func()->iremotePort    = itcpSocket->peerPort();

    } else if ( ibackendType == ELocalSocket ) {
        irequest->d_func()->iremoteAddress = ilocalSocket->fullServerName();
        irequest->d_func()->iremotePort    = 0; // not used in local sockets
    }

    Q_ASSERT( iresponse == nullptr );
    iresponse  = new QHttpResponse(q_func());

    if ( parser->http_major < 1 || parser->http_minor < 1  )
        iresponse->d_func()->ikeepAlive = false;

    // we are good to go!
    if ( ihandler )
        ihandler(irequest, iresponse);
    else
        emit q_ptr->newRequest(irequest, iresponse);

    return 0;
}

int
QHttpConnectionPrivate::body(http_parser*, const char* at, size_t length) {
    CHECK_FOR_DISCONNECTED

    if ( irequest->d_func()->icollectCapacity != 0 ) {
        int currentLength = irequest->d_func()->icollectedData.length();
        if ( (currentLength + (int)length) < irequest->d_func()->icollectCapacity )
            irequest->d_func()->icollectedData.append(at, length);
        else
            messageComplete(nullptr); // no other data will be read.

        return 0;
    }

    if ( irequest->idataHandler )
        irequest->idataHandler(QByteArray(at, length));
    else
        emit irequest->data(QByteArray(at, length));

    return 0;
}

int
QHttpConnectionPrivate::messageComplete(http_parser*) {
    CHECK_FOR_DISCONNECTED

    //prevents double messageComplete calls
    if ( irequest->d_func()->isuccessful )
        return 0;

    irequest->d_func()->isuccessful = true;

    if ( irequest->iendHandler )
        irequest->iendHandler();
    else
        emit irequest->end();

    return 0;
}



///////////////////////////////////////////////////////////////////////////////
#if defined(USE_CUSTOM_URL_CREATOR)
///////////////////////////////////////////////////////////////////////////////
/* URL Utilities */
#define HAS_URL_FIELD(info, field) (info.field_set &(1 << (field)))

#define GET_FIELD(data, info, field)                                                               \
    QString::fromLatin1(data + info.field_data[field].off, info.field_data[field].len)

#define CHECK_AND_GET_FIELD(data, info, field)                                                     \
    (HAS_URL_FIELD(info, field) ? GET_FIELD(data, info, field) : QString())

QUrl
QHttpConnectionPrivate::createUrl(const char *urlData, const http_parser_url &urlInfo) {
    QUrl url;
    url.setScheme(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_SCHEMA));
    url.setHost(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_HOST));
    // Port is dealt with separately since it is available as an integer.
    url.setPath(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_PATH));
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    url.setQuery(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_QUERY));
#else
    if (HAS_URL_FIELD(urlInfo, UF_QUERY)) {
        url.setEncodedQuery(QByteArray(urlData + urlInfo.field_data[UF_QUERY].off,
                                       urlInfo.field_data[UF_QUERY].len));
    }
#endif
    url.setFragment(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_FRAGMENT));
    url.setUserInfo(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_USERINFO));

    if (HAS_URL_FIELD(urlInfo, UF_PORT))
        url.setPort(urlInfo.port);

    return url;
}

#undef CHECK_AND_SET_FIELD
#undef GET_FIELD
#undef HAS_URL_FIELD
///////////////////////////////////////////////////////////////////////////////
#endif // defined(USE_CUSTOM_URL_CREATOR)

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
