#ifndef QHTTPRESPONSE_PRIVATE_HPP
#define QHTTPRESPONSE_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////
#include "qhttpresponse.hpp"
#include "qhttpserver.hpp"
#include "qhttpconnection.hpp"

#include <QDateTime>
#include <QLocale>
#include <QTcpSocket>

///////////////////////////////////////////////////////////////////////////////
class QHttpResponse::Private
{
    QHttpResponse*      iparent;

public:
    explicit    Private(QHttpResponse* parent, QTcpSocket* sok) : iparent(parent) {
        reset();

        m_socket    = sok;

        QObject::connect(m_socket, &QTcpSocket::bytesWritten, [this](qint64 byteCount){
            updateWriteCount(byteCount);
        });

        QObject::connect(iparent,     &QHttpResponse::done,
                         [this](bool wasTheLastResponse){
            if ( wasTheLastResponse )
                m_socket->disconnectFromHost();
        });
    }

    void        reset() {
        m_socket                    = nullptr;

        m_headerWritten             = false;
        m_sentConnectionHeader      = false;
        m_sentContentLengthHeader   = false;
        m_sentTransferEncodingHeader= false;
        m_sentDate                  = false;
        m_keepAlive                 = false;
        m_last                      = false;
        m_useChunkedEncoding        = false;
        m_finished                  = false;

        m_transmitLen = m_transmitPos = 0;
    }

    void        writeHeaders();
    void        writeHeader(const QByteArray& field, const QByteArray& value);

    void        write(const QByteArray &data) {
        m_socket->write(data);
        m_transmitLen += data.size();
    }

    void        updateWriteCount(qint64 count) {
        Q_ASSERT(m_transmitPos + count <= m_transmitLen);

        m_transmitPos += count;

        if ( m_transmitPos == m_transmitLen ) {
            m_transmitLen = 0;
            m_transmitPos = 0;
            emit iparent->allBytesWritten();
        }
    }


public:
    THeaderHash          m_headers;
    QTcpSocket*          m_socket;

    bool                 m_headerWritten;
    bool                 m_sentConnectionHeader;
    bool                 m_sentContentLengthHeader;
    bool                 m_sentTransferEncodingHeader;
    bool                 m_sentDate;
    bool                 m_keepAlive;
    bool                 m_last;
    bool                 m_useChunkedEncoding;
    bool                 m_finished;

    // Keep track of transmit buffer status
    qint64               m_transmitLen;
    qint64               m_transmitPos;


};

///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPRESPONSE_PRIVATE_HPP
