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
class QHttpResponsePrivate : public HttpResponseBase
{
    Q_DECLARE_PUBLIC(QHttpResponse)

public:
    explicit    QHttpResponsePrivate(QTcpSocket* sok, QHttpResponse* q)
        : q_ptr(q) {
        reset();

        isocket    = sok;

        QObject::connect(isocket, &QTcpSocket::bytesWritten, [this](qint64 byteCount){
            updateWriteCount(byteCount);
        });

        QObject::connect(isocket, &QTcpSocket::disconnected, [this](){
            ifinished   = true;
            q_func()->deleteLater();
        });

        QObject::connect(q_func(),     &QHttpResponse::done,
                         [this](bool wasTheLastResponse){
            if ( wasTheLastResponse )
                isocket->disconnectFromHost();
        });

        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpResponsePrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void        reset() {
        isocket                    = nullptr;

        iheaderWritten             = false;
        ikeepAlive                 = false;
        ifinished                  = false;

        itransmitLen = itransmitPos = 0;
    }

    void        ensureWritingHeaders();

    void        writeHeaders();

protected:
    void        writeHeader(const QByteArray& field, const QByteArray& value) {
        QByteArray buffer = QByteArray(field)
                            .append(": ")
                            .append(value)
                            .append("\r\n");
        write(buffer);
    }

    void        updateWriteCount(qint64 count) {
        Q_ASSERT(itransmitPos + count <= itransmitLen);

        itransmitPos += count;

        if ( itransmitPos == itransmitLen ) {
            itransmitLen = 0;
            itransmitPos = 0;
            emit q_func()->allBytesWritten();
        }
    }

    void        write(const QByteArray &data) {
        isocket->write(data);
        itransmitLen += data.size();
    }

public:
    QTcpSocket*          isocket;

    bool                 iheaderWritten;
    bool                 ikeepAlive;
    bool                 ifinished;

    // Keep track of transmit buffer status
    qint64               itransmitLen;
    qint64               itransmitPos;

protected:
    QHttpResponse*       q_ptr;

};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPRESPONSE_PRIVATE_HPP
