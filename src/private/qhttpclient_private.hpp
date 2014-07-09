#ifndef QHTTPCLIENT_PRIVATE_HPP
#define QHTTPCLIENT_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpclient.hpp"
#include "qhttpclientrequest_private.hpp"
#include "qhttpclientresponse_private.hpp"

#include <QPointer>
#include <QBasicTimer>
#include <QFile>

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

public:
    explicit     QHttpClientPrivate(QHttpClient* q) : HttpParserBase(HTTP_RESPONSE), q_ptr(q) {
        istatus         = ESTATUS_BAD_REQUEST;
        itimeOut        = 0;

        QHTTP_LINE_DEEPLOG
    }

    virtual     ~QHttpClientPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void         initialize() {
        Q_ASSERT(q_func());

        isocket         = new QTcpSocket(q_func());

        QObject::connect(isocket,  &QTcpSocket::connected, [this]{
#               if QHTTP_MESSAGES_LOG > 0
                iinputBuffer.clear();
#               endif

            ilastRequest  = new QHttpRequest(isocket);
            ilastRequest->d_func()->imethod  = ilastMethod;
            ilastRequest->d_func()->iurl     = ilastUrl;

            if ( itimeOut > 0 )
                itimer.start(itimeOut, Qt::CoarseTimer, q_func());

            emit q_func()->onRequestReady(ilastRequest);
        });

        QObject::connect(isocket,  &QTcpSocket::readyRead, [this](){
            while ( isocket->bytesAvailable() > 0 ) {
                char buffer[4097] = {0};
                size_t readLength = isocket->read(buffer, 4096);

#               if QHTTP_MESSAGES_LOG > 0
                iinputBuffer.append(buffer);
#               endif

                parse(buffer, readLength);
            }
        });

        QObject::connect(isocket,  &QTcpSocket::disconnected, [this]{
#           if QHTTP_MESSAGES_LOG > 0
            QFile f("/tmp/qhttpclient-incomming.log");
            if ( f.open(QIODevice::Append | QIODevice::WriteOnly) ) {
                f.write(iinputBuffer);
                f.write("\n---------------------\n");
                f.flush();
            }
#           endif

            puts("http client socket has been disconnected.");
            emit q_func()->disconnected();
        });
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

    QPointer<QHttpRequest>  ilastRequest;
    QPointer<QHttpResponse> ilastResponse;

#   if QHTTP_MESSAGES_LOG > 0
    QByteArray              iinputBuffer;
#   endif
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////

#endif // QHTTPCLIENT_PRIVATE_HPP
