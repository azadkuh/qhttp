#ifndef QHTTPRESPONSE_PRIVATE_HPP
#define QHTTPRESPONSE_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////
#include "qhttpresponse.hpp"
#include "qhttpserver.hpp"
#include "qhttpconnection.hpp"

#include <QDateTime>
#include <QLocale>

///////////////////////////////////////////////////////////////////////////////
class QHttpResponse::Private
{
public:
    explicit Private(QHttpConnection* conn) : m_connection(conn),
        m_headerWritten(false),
        m_sentConnectionHeader(false),
        m_sentContentLengthHeader(false),
        m_sentTransferEncodingHeader(false),
        m_sentDate(false),
        m_keepAlive(true),
        m_last(false),
        m_useChunkedEncoding(false),
        m_finished(false){
    }

    void        writeHeaders();
    void        writeHeader(const char *field, const QByteArray& value);


public:

    QHttpConnection*     m_connection;
    THeaderHash          m_headers;

    bool                 m_headerWritten;
    bool                 m_sentConnectionHeader;
    bool                 m_sentContentLengthHeader;
    bool                 m_sentTransferEncodingHeader;
    bool                 m_sentDate;
    bool                 m_keepAlive;
    bool                 m_last;
    bool                 m_useChunkedEncoding;
    bool                 m_finished;
};

///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPRESPONSE_PRIVATE_HPP
