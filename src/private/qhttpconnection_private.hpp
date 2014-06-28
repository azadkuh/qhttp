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

    QTcpSocket*             m_socket;
    http_parser*            m_parser;
    http_parser_settings*   m_parserSettings;

    // Since there can only be one request at any time even with pipelining.
    QHttpRequest*           m_request;      ///< latest request
    QHttpResponse*          m_response;     ///< latest response

    QByteArray              m_currentUrl;
    // The ones we are reading in from the parser
    THeaderHash             m_currentHeaders;
    QByteArray              m_currentHeaderField;
    QByteArray              m_currentHeaderValue;

    QBasicTimer             m_timer;

#   if QHTTPSERVER_MESSAGES_LOG > 0
    QByteArray              m_inputBuffer;
#   endif

public:
    explicit     Private(qintptr handle, QHttpConnection* p, quint32 timeOut) : iparent(p),
        m_socket(nullptr),
        m_parser(nullptr),
        m_parserSettings(nullptr),
        m_request(nullptr),
        m_response(nullptr) {

        // create http_parser object
        m_parser = (http_parser *)malloc(sizeof(http_parser)); {
            http_parser_init(m_parser, HTTP_REQUEST);

            m_parserSettings = new http_parser_settings();
            m_parserSettings->on_message_begin    = Private::onMessageBegin;
            m_parserSettings->on_url              = Private::onUrl;
            m_parserSettings->on_header_field     = Private::onHeaderField;
            m_parserSettings->on_header_value     = Private::onHeaderValue;
            m_parserSettings->on_headers_complete = Private::onHeadersComplete;
            m_parserSettings->on_body             = Private::onBody;
            m_parserSettings->on_message_complete = Private::onMessageComplete;
        }
        m_parser->data  = iparent;

        if ( timeOut != 0 )
            m_timer.start(timeOut, iparent);

        m_socket        = new QTcpSocket(iparent);
        m_socket->setSocketDescriptor(handle);

        QObject::connect(m_socket, &QTcpSocket::readyRead, [this](){
            parseRequest();
        });

        QObject::connect(m_socket, &QTcpSocket::disconnected, [this](){
            m_socket->deleteLater();
            iparent->deleteLater();
        });
    }

    ~Private() {
        if ( m_parser != nullptr ) {
            free(m_parser);
            m_parser = nullptr;
        }

        if ( m_parserSettings != nullptr ) {
            delete m_parserSettings;
            m_parserSettings = nullptr;
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
