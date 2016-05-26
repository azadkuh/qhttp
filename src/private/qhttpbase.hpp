/** base classes for private implementations.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPBASE_HPP
#define QHTTPBASE_HPP

#include "qhttpfwd.hpp"

#include "qsocket.hpp"
#include <QHostAddress>
#include <QBasicTimer>

#include "http-parser/http_parser.h"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace details {
///////////////////////////////////////////////////////////////////////////////

struct HttpBase
{
    QString     iversion;
    THeaderHash iheaders;
}; // struct HttpBase

///////////////////////////////////////////////////////////////////////////////

struct HttpRequestBase : public HttpBase
{
    QUrl        iurl;
    THttpMethod imethod;
}; // HttpRequestBase

///////////////////////////////////////////////////////////////////////////////

struct HttpResponseBase : public HttpBase
{
    TStatusCode istatus = ESTATUS_BAD_REQUEST;

    HttpResponseBase() { iversion = "1.1"; }
}; // HttpResponseBase

///////////////////////////////////////////////////////////////////////////////

// usage in client::QHttpRequest, server::QHttpResponse
template<class TBase, class TImpl>
class HttpWriter : public TBase
{
public:
    bool            addHeader(const QByteArray &field, const QByteArray &value) {
        if ( ifinished )
            return false;

        TBase::iheaders.insert(field.toLower(), value);
        return true;
    }

    bool            writeHeader(const QByteArray& field, const QByteArray& value) {
        if ( ifinished )
            return false;

        QByteArray buffer = QByteArray(field)
                            .append(": ")
                            .append(value)
                            .append("\r\n");

        isocket.writeRaw(buffer);
        return true;
    }

    bool            writeData(const QByteArray& data) {
        if ( ifinished )
            return false;

        ensureWritingHeaders();
        isocket.writeRaw(data);
        return true;
    }

    bool            endPacket(const QByteArray& data) {
        if ( !writeData(data) )
            return false;

        isocket.flush();
        ifinished = true;
        return true;
    }

    void            ensureWritingHeaders() {
        if ( ifinished    ||    iheaderWritten )
            return;

        TImpl* me = static_cast<TImpl*>(this);
        isocket.writeRaw(me->makeTitle());
        writeHeaders();

        iheaderWritten = true;
    }

    void            writeHeaders(bool doFlush = false) {
        if ( ifinished    ||    iheaderWritten )
            return;

        if ( TBase::iheaders.keyHasValue("connection", "keep-alive") )
            ikeepAlive = true;
        else
            TBase::iheaders.insert("connection", "close");

        TImpl* me = static_cast<TImpl*>(this);
        me->prepareHeadersToWrite();


        for ( auto cit = TBase::iheaders.constBegin(); cit != TBase::iheaders.constEnd(); cit++ ) {
            const QByteArray& field = cit.key();
            const QByteArray& value = cit.value();

            writeHeader(field, value);
        }

        isocket.writeRaw("\r\n");
        if ( doFlush )
            isocket.flush();
    }

public:
    QSocket         isocket;

    bool            ifinished = false;
    bool            iheaderWritten = false;
    bool            ikeepAlive = false;
};

///////////////////////////////////////////////////////////////////////////////

// usage in client::QHttpClient, server::QHttpConnection
template<class TImpl>
class HttpParser
{
public:
    explicit     HttpParser(http_parser_type type) {
        // create http_parser object
        iparser.data  = static_cast<TImpl*>(this);
        http_parser_init(&iparser, type);

        memset(&iparserSettings, 0, sizeof(http_parser_settings));
        iparserSettings.on_message_begin    = onMessageBegin;
        iparserSettings.on_url              = onUrl;
        iparserSettings.on_status           = onStatus;
        iparserSettings.on_header_field     = onHeaderField;
        iparserSettings.on_header_value     = onHeaderValue;
        iparserSettings.on_headers_complete = onHeadersComplete;
        iparserSettings.on_body             = onBody;
        iparserSettings.on_message_complete = onMessageComplete;
    }

    size_t       parse(const char* data, size_t length) {
        return http_parser_execute(&iparser,
                                   &iparserSettings,
                                   data,
                                   length);
    }

public: // callback functions for http_parser_settings
    static int   onMessageBegin(http_parser* parser) {
        TImpl *me = static_cast<TImpl*>(parser->data);
        return me->messageBegin(parser);
    }

    static int   onUrl(http_parser* parser, const char* at, size_t length) {
        TImpl *me = static_cast<TImpl*>(parser->data);
        return me->url(parser, at, length);
    }

    static int   onStatus(http_parser* parser, const char* at, size_t length) {
        TImpl *me = static_cast<TImpl*>(parser->data);
        return me->status(parser, at, length);
    }

    static int   onHeaderField(http_parser* parser, const char* at, size_t length) {
        TImpl *me = static_cast<TImpl*>(parser->data);
        return me->headerField(parser, at, length);
    }

    static int   onHeaderValue(http_parser* parser, const char* at, size_t length) {
        TImpl *me = static_cast<TImpl*>(parser->data);
        return me->headerValue(parser, at, length);
    }

    static int   onHeadersComplete(http_parser* parser) {
        TImpl *me = static_cast<TImpl*>(parser->data);
        return me->headersComplete(parser);
    }

    static int   onBody(http_parser* parser, const char* at, size_t length) {
        TImpl *me = static_cast<TImpl*>(parser->data);
        return me->body(parser, at, length);
    }

    static int   onMessageComplete(http_parser* parser) {
        TImpl *me = static_cast<TImpl*>(parser->data);
        return me->messageComplete(parser);
    }


protected:
    // The ones we are reading in from the parser
    QByteArray              itempHeaderField;
    QByteArray              itempHeaderValue;
    // if connection has a timeout, these fields will be used
    quint32                 itimeOut = 0;
    QBasicTimer             itimer;
    // uniform socket object
    QSocket                 isocket;
    // if connection should persist
    bool                    ikeepAlive = false;



protected:
    http_parser             iparser;
    http_parser_settings    iparserSettings;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace details
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPBASE_HPP
