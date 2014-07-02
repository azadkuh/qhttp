#ifndef QHTTPCONNECTION_PRIVATE_HPP
#define QHTTPCONNECTION_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpconnection.hpp"

#include "qhttprequest.hpp"
#include "qhttpresponse.hpp"

#include "private/qhttprequest_private.hpp"
#include "private/qhttpresponse_private.hpp"

#include <QBasicTimer>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHttpConnection::Private : public HttpParserBase<QHttpConnection::Private>
{
public:
    QHttpConnection*        iparent;

    // Since there can only be one request at any time even with pipelining.
    QHttpRequest*           irequest;      ///< latest request
    QHttpResponse*          iresponse;     ///< latest response

    QBasicTimer             itimer;

#   if QHTTPSERVER_MESSAGES_LOG > 0
    QByteArray              iinputBuffer;
#   endif

public:
    explicit     Private(qintptr handle, QHttpConnection* p, quint32 timeOut)
        : HttpParserBase(HTTP_REQUEST), iparent(p),
        irequest(nullptr),
        iresponse(nullptr) {


        if ( timeOut != 0 )
            itimer.start(timeOut, iparent);

        isocket        = new QTcpSocket(iparent);
        isocket->setSocketDescriptor(handle);

        QObject::connect(isocket, &QTcpSocket::readyRead, [this](){
            while (isocket->bytesAvailable()) {
                char buffer[4096] = {0};
                size_t readLength = isocket->read(buffer, 4095);

        #       if QHTTPSERVER_MESSAGES_LOG > 0
                iinputBuffer.append(buffer);
        #       endif

                parse(buffer, readLength);
            }
        });

        QObject::connect(isocket, &QTcpSocket::disconnected, [this](){
            emit iparent->dropped();
            iparent->deleteLater();
        });
    }

    ~Private() {
    }

public:
    int          messageBegin(http_parser *parser);
    int          url(http_parser *parser, const char *at, size_t length);
    int          headerField(http_parser *parser, const char *at, size_t length);
    int          headerValue(http_parser *parser, const char *at, size_t length);
    int          headersComplete(http_parser *parser);
    int          body(http_parser *parser, const char *at, size_t length);
    int          messageComplete(http_parser *parser);

#ifdef USE_CUSTOM_URL_CREATOR
public:
    static QUrl  createUrl(const char *urlData, const http_parser_url &urlInfo);
#endif // USE_CUSTOM_URL_CREATOR
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPCONNECTION_PRIVATE_HPP
