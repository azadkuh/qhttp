#ifndef QHTTPBASE_HPP
#define QHTTPBASE_HPP

#include "qhttpserverfwd.hpp"

#include <QTcpSocket>
#include <QHostAddress>
#include <QUrl>

#include "http-parser/http_parser.h"

///////////////////////////////////////////////////////////////////////////////

class HttpBase
{
public:
    THeaderHash         iheaders;
};

///////////////////////////////////////////////////////////////////////////////

class HttpRequestBase : public HttpBase
{
public:
    QUrl                iurl;
    THttpMethod         imethod;
};

///////////////////////////////////////////////////////////////////////////////

class HttpResponseBase : public HttpBase
{
public:
    int                 istatus;
};

///////////////////////////////////////////////////////////////////////////////
template<class T>
class HttpParserBase
{
public:
    HttpParserBase(http_parser_type type) : isocket(nullptr),
        iparser(nullptr), iparserSettings(nullptr) {
        // create http_parser object
        iparser         = new http_parser();
        iparser->data   = this;
        http_parser_init(iparser, type);

        iparserSettings = new http_parser_settings();
        iparserSettings->on_message_begin    = onMessageBegin;
        iparserSettings->on_url              = onUrl;
        iparserSettings->on_header_field     = onHeaderField;
        iparserSettings->on_header_value     = onHeaderValue;
        iparserSettings->on_headers_complete = onHeadersComplete;
        iparserSettings->on_body             = onBody;
        iparserSettings->on_message_complete = onMessageComplete;
    }

    ~HttpParserBase() {
        if ( iparser != nullptr ) {
            delete iparser;
            iparser = nullptr;
        }

        if ( iparserSettings != nullptr ) {
            delete iparserSettings;
            iparserSettings = nullptr;
        }
    }

    void         parse(const char* data, size_t length) {
        http_parser_execute(iparser, iparserSettings,
                            data, length);
    }

public: // callback functions for http_parser_settings
    static int   onMessageBegin(http_parser *parser) {
        T *me = static_cast<T*>(parser->data);
        return me->messageBegin(parser);
    }

    static int   onUrl(http_parser *parser, const char *at, size_t length) {
        T *me = static_cast<T*>(parser->data);
        return me->url(parser, at, length);
    }

    static int   onHeaderField(http_parser *parser, const char *at, size_t length) {
        T *me = static_cast<T*>(parser->data);
        return me->headerField(parser, at, length);
    }

    static int   onHeaderValue(http_parser *parser, const char *at, size_t length) {
        T *me = static_cast<T*>(parser->data);
        return me->headerValue(parser, at, length);
    }

    static int   onHeadersComplete(http_parser *parser) {
        T *me = static_cast<T*>(parser->data);
        return me->headersComplete(parser);
    }

    static int   onBody(http_parser *parser, const char *at, size_t length) {
        T *me = static_cast<T*>(parser->data);
        return me->body(parser, at, length);
    }

    static int   onMessageComplete(http_parser *parser) {
        T *me = static_cast<T*>(parser->data);
        return me->messageComplete(parser);
    }

public:
    QTcpSocket*             isocket;

    // The ones we are reading in from the parser
    QByteArray              itempHeaderField;
    QByteArray              itempHeaderValue;
    QByteArray              itempUrl;

private:
    http_parser*            iparser;
    http_parser_settings*   iparserSettings;
};

///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPBASE_HPP
