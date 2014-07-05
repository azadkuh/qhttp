#ifndef QHTTPRESPONSE_PRIVATE_HPP
#define QHTTPRESPONSE_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////
#include "qhttpbase.hpp"
#include "qhttpresponse.hpp"
#include "qhttpserver.hpp"
#include "qhttpconnection.hpp"

#include <QDateTime>
#include <QLocale>
#include <QTcpSocket>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHttpResponse::Private : public HttpResponseBase
{
    QHttpResponse*      iparent;

public:
    explicit    Private(QHttpResponse* parent, QTcpSocket* sok) : iparent(parent) {
        reset();

        isocket    = sok;

        QObject::connect(isocket, &QTcpSocket::bytesWritten, [this](qint64 byteCount){
            updateWriteCount(byteCount);
        });

        QObject::connect(isocket, &QTcpSocket::disconnected, [this](){
            ifinished   = true;
            iparent->deleteLater();
        });

        QObject::connect(iparent,     &QHttpResponse::done,
                         [this](bool wasTheLastResponse){
            if ( wasTheLastResponse )
                isocket->disconnectFromHost();
        });

        QHTTP_LINE_DEEPLOG
    }

    virtual    ~Private() {
        QHTTP_LINE_DEEPLOG
    }

    void        reset() {
        isocket                    = nullptr;

        iheaderWritten             = false;
        ikeepAlive                 = false;
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
    QTcpSocket*          isocket;

    bool                 iheaderWritten;
    bool                 ikeepAlive;
    bool                 ifinished;

    // Keep track of transmit buffer status
    qint64               itransmitLen;
    qint64               itransmitPos;


};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPRESPONSE_PRIVATE_HPP
