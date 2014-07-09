#ifndef QHTTPSERVER_CONNECTION_PRIVATE_HPP
#define QHTTPSERVER_CONNECTION_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpserverconnection.hpp"

#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include "private/qhttpserverrequest_private.hpp"
#include "private/qhttpserverresponse_private.hpp"

#include <QBasicTimer>
#include <QFile>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHttpConnectionPrivate  : public HttpParserBase<QHttpConnectionPrivate>
{
    Q_DECLARE_PUBLIC(QHttpConnection)

public:
    QByteArray              itempUrl;

    // Since there can only be one request at any time even with pipelining.
    QHttpRequest*           irequest;      ///< latest request
    QHttpResponse*          iresponse;     ///< latest response

    QBasicTimer             itimer;

#   if QHTTP_MESSAGES_LOG > 0
    QByteArray              iinputBuffer;
#   endif

public:
    explicit    QHttpConnectionPrivate(QHttpConnection* q)
        : HttpParserBase(HTTP_REQUEST), q_ptr(q) {
        irequest       = nullptr;
        iresponse      = nullptr;
        isocket        = q;

        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpConnectionPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void        initialize() {

        QObject::connect(isocket, &QTcpSocket::readyRead, [this](){
            while (isocket->bytesAvailable()) {
                char buffer[4096] = {0};
                size_t readLength = isocket->read(buffer, 4095);

#              if QHTTP_MESSAGES_LOG > 0
                iinputBuffer.append(buffer);
#              endif

                parse(buffer, readLength);
            }
        });

        QObject::connect(isocket, &QTcpSocket::disconnected, [this](){

#           if QHTTP_MESSAGES_LOG > 0
            QFile f("/tmp/incomingMessages.log");
            if ( f.open(QIODevice::Append | QIODevice::WriteOnly) ) {
                f.write(iinputBuffer);
                f.write("\n---------------------\n");
                f.flush();
            }
#           endif
            q_ptr->deleteLater();
        });
    }

public:
    int          messageBegin(http_parser* parser);
    int          url(http_parser* parser, const char* at, size_t length);
    int          status(http_parser*, const char*, size_t) {
        return 0;   // not used in parsing incoming request.
    }
    int          headerField(http_parser* parser, const char* at, size_t length);
    int          headerValue(http_parser* parser, const char* at, size_t length);
    int          headersComplete(http_parser* parser);
    int          body(http_parser* parser, const char* at, size_t length);
    int          messageComplete(http_parser* parser);

#ifdef USE_CUSTOM_URL_CREATOR
public:
    static QUrl  createUrl(const char *urlData, const http_parser_url &urlInfo);
#endif // USE_CUSTOM_URL_CREATOR

protected:
    QHttpConnection* const q_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPSERVER_CONNECTION_PRIVATE_HPP
