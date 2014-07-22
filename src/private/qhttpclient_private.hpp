/** private imeplementation.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

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
    THttpMethod     ilastMethod;
    QUrl            ilastUrl;

    quint32         itimeOut = 0;
    QBasicTimer     itimer;

public:
    explicit     QHttpClientPrivate(QHttpClient* q) : HttpParserBase(HTTP_RESPONSE), q_ptr(q) {
        QHTTP_LINE_DEEPLOG
    }

    virtual     ~QHttpClientPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void         initialize() {
        isocket = static_cast<QTcpSocket*>(q_func());

        QObject::connect(isocket,  &QTcpSocket::connected, [this](){
            onConnected();
        });
        QObject::connect(isocket,  &QTcpSocket::readyRead, [this](){
            onReadyRead();
        });
        QObject::connect(isocket,  &QTcpSocket::disconnected, [this](){
#           if QHTTP_MESSAGES_LOG > 0
            QFile f("/tmp/qhttpclient-incomming.log");
            if ( f.open(QIODevice::Append | QIODevice::WriteOnly) ) {
                f.write(iinputBuffer);
                f.write("\n---------------------\n");
                f.flush();
            }
#           endif

            q_func()->deleteLater();
        });


#       if QHTTP_MESSAGES_LOG > 0
        iinputBuffer.reserve(1024);
#       endif
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
    void         onConnected();
    void         onReadyRead();

protected:
    QHttpClient* const      q_ptr;

    QHttpResponse*          ilastResponse = nullptr;

#   if QHTTP_MESSAGES_LOG > 0
    QByteArray              iinputBuffer;
#   endif
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////

#endif // QHTTPCLIENT_PRIVATE_HPP
