#ifndef QHTTPCONNECTION_PRIVATE_HPP
#define QHTTPCONNECTION_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpconnection.hpp"
#include "http-parser/http_parser.h"

#include "qhttprequest.hpp"
#include "qhttpresponse.hpp"

#include "private/qhttprequest_private.hpp"
#include "private/qhttpresponse_private.hpp"

#include <QTcpSocket>
#include <QHostAddress>
#include <QBasicTimer>

///////////////////////////////////////////////////////////////////////////////
class QHttpConnection::Private
{
public:
    QHttpConnection*        iparent;

    QTcpSocket*             isocket;
    http_parser*            iparser;
    http_parser_settings*   iparserSettings;

    // Since there can only be one request at any time even with pipelining.
    QHttpRequest*           irequest;      ///< latest request
    QHttpResponse*          iresponse;     ///< latest response

    QByteArray              itempUrl;
    // The ones we are reading in from the parser
    QByteArray              itempHeaderField;
    QByteArray              itempHeaderValue;

    QBasicTimer             itimer;

#   if QHTTPSERVER_MESSAGES_LOG > 0
    QByteArray              iinputBuffer;
#   endif

public:
    explicit     Private(qintptr handle, QHttpConnection* p, quint32 timeOut) : iparent(p),
        isocket(nullptr),
        iparser(nullptr),
        iparserSettings(nullptr),
        irequest(nullptr),
        iresponse(nullptr) {

        // create http_parser object
        iparser = (http_parser *)malloc(sizeof(http_parser)); {
            http_parser_init(iparser, HTTP_REQUEST);

            iparserSettings = new http_parser_settings();
            iparserSettings->on_message_begin    = Private::onMessageBegin;
            iparserSettings->on_url              = Private::onUrl;
            iparserSettings->on_header_field     = Private::onHeaderField;
            iparserSettings->on_header_value     = Private::onHeaderValue;
            iparserSettings->on_headers_complete = Private::onHeadersComplete;
            iparserSettings->on_body             = Private::onBody;
            iparserSettings->on_message_complete = Private::onMessageComplete;
        }
        iparser->data  = iparent;

        if ( timeOut != 0 )
            itimer.start(timeOut, iparent);

        isocket        = new QTcpSocket(iparent);
        isocket->setSocketDescriptor(handle);

        QObject::connect(isocket, &QTcpSocket::readyRead, [this](){
            parseRequest();
        });

        QObject::connect(isocket, &QTcpSocket::disconnected, [this](){
            emit iparent->dropped();
            iparent->deleteLater();
        });
    }

    ~Private() {
        if ( iparser != nullptr ) {
            free(iparser);
            iparser = nullptr;
        }

        if ( iparserSettings != nullptr ) {
            delete iparserSettings;
            iparserSettings = nullptr;
        }
    }

public:
    void         parseRequest();

public:
    int          messageBegin(http_parser *parser);
    int          url(http_parser *parser, const char *at, size_t length);
    int          headerField(http_parser *parser, const char *at, size_t length);
    int          headerValue(http_parser *parser, const char *at, size_t length);
    int          headersComplete(http_parser *parser);
    int          body(http_parser *parser, const char *at, size_t length);
    int          messageComplete(http_parser *parser);

public: // callback functions for http_parser_settings
    static int   onMessageBegin(http_parser *parser) {
        QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
        return theConnection->pimp->messageBegin(parser);
    }

    static int   onUrl(http_parser *parser, const char *at, size_t length) {
        QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
        return theConnection->pimp->url(parser, at, length);
    }

    static int   onHeaderField(http_parser *parser, const char *at, size_t length) {
        QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
        return theConnection->pimp->headerField(parser, at, length);
    }

    static int   onHeaderValue(http_parser *parser, const char *at, size_t length) {
        QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
        return theConnection->pimp->headerValue(parser, at, length);
    }

    static int   onHeadersComplete(http_parser *parser) {
        QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
        return theConnection->pimp->headersComplete(parser);
    }

    static int   onBody(http_parser *parser, const char *at, size_t length) {
        QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
        return theConnection->pimp->body(parser, at, length);
    }

    static int   onMessageComplete(http_parser *parser) {
        QHttpConnection *theConnection = static_cast<QHttpConnection *>(parser->data);
        return theConnection->pimp->messageComplete(parser);
    }

public:
    static QUrl  createUrl(const char *urlData, const http_parser_url &urlInfo);
};

///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPCONNECTION_PRIVATE_HPP
