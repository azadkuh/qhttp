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

        isocket    = sok;

        QObject::connect(isocket, &QTcpSocket::bytesWritten, [this](qint64 byteCount){
            updateWriteCount(byteCount);
        });

        QObject::connect(iparent,     &QHttpResponse::done,
                         [this](bool wasTheLastResponse){
            if ( wasTheLastResponse )
                isocket->disconnectFromHost();
        });
    }

    void        reset() {
        isocket                    = nullptr;

        iheaderWritten             = false;
        isentConnectionHeader      = false;
        isentContentLengthHeader   = false;
        isentTransferEncodingHeader= false;
        isentDate                  = false;
        ikeepAlive                 = false;
        ilast                      = false;
        iuseChunkedEncoding        = false;
        ifinished                  = false;

        itransmitLen = itransmitPos = 0;
    }

    void        writeHeaders();
    void        writeHeader(const QByteArray& field, const QByteArray& value);

    void        write(const QByteArray &data) {
        isocket->write(data);
        itransmitLen += data.size();
    }

    void        updateWriteCount(qint64 count) {
        Q_ASSERT(itransmitPos + count <= itransmitLen);

        itransmitPos += count;

        if ( itransmitPos == itransmitLen ) {
            itransmitLen = 0;
            itransmitPos = 0;
            emit iparent->allBytesWritten();
        }
    }


public:
    THeaderHash          iheaders;
    QTcpSocket*          isocket;

    bool                 iheaderWritten;
    bool                 isentConnectionHeader;
    bool                 isentContentLengthHeader;
    bool                 isentTransferEncodingHeader;
    bool                 isentDate;
    bool                 ikeepAlive;
    bool                 ilast;
    bool                 iuseChunkedEncoding;
    bool                 ifinished;

    // Keep track of transmit buffer status
    qint64               itransmitLen;
    qint64               itransmitPos;


};

///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPRESPONSE_PRIVATE_HPP
