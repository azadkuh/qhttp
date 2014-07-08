#ifndef QHTTPCLIENT_PRIVATE_HPP
#define QHTTPCLIENT_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpclient.hpp"
#include "qhttpclientrequest_private.hpp"
#include "qhttpclientresponse_private.hpp"

#include <QBasicTimer>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////

class QHttpClientPrivate : public HttpParserBase<QHttpClientPrivate>
{
    Q_DECLARE_PUBLIC(QHttpClient)

public:
    TStatusCode     istatus;
    THttpMethod     ilastMethod;
    QUrl            ilastUrl;

    quint32         itimeOut;
    QBasicTimer     itimer;

    QHttpRequest*   ilastRequest;
    QHttpResponse*  ilastResponse;

public:
    explicit    QHttpClientPrivate(QHttpClient* q)
        : HttpParserBase(HTTP_RESPONSE), itimeOut(0), q_ptr(q) {
        ilastRequest    = nullptr;
        ilastResponse   = nullptr;

        isocket         = new QTcpSocket(q_ptr);

        QObject::connect(isocket,  &QTcpSocket::connected, [this]{
            ilastRequest  = new QHttpRequest(isocket);
            ilastRequest->d_func()->imethod  = ilastMethod;
            ilastRequest->d_func()->iurl     = ilastUrl;

            if ( itimeOut > 0 )
                itimer.start(itimeOut, Qt::CoarseTimer, q_func());

            emit q_func()->connected(ilastRequest);
        });

        QObject::connect(isocket,  &QTcpSocket::readyRead, [this](){
            while ( isocket->bytesAvailable() > 0 ) {
                char buffer[4097] = {0};
                size_t readLength = isocket->read(buffer, 4096);

                parse(buffer, readLength);
            }
        });

        QObject::connect(isocket,  &QTcpSocket::disconnected,
                         q_func(), &QHttpClient::disconnected
                         );

        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpClientPrivate() {
        QHTTP_LINE_DEEPLOG
    }

public:
    int          messageBegin(http_parser* parser);
    int          url(http_parser*, const char*, size_t) {
        return 0; // not used in parsing incoming respone.
    }
    int          status(http_parser* parser, const char* at, size_t length) ;
    int          headerField(http_parser* parser, const char* at, size_t length);
    int          headerValue(http_parser* parser, const char* at, size_t length);
    int          headersComplete(http_parser* parser);
    int          body(http_parser* parser, const char* at, size_t length);
    int          messageComplete(http_parser* parser);

protected:
    QHttpClient* const      q_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////

#endif // QHTTPCLIENT_PRIVATE_HPP
