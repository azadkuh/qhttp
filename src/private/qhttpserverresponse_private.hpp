#ifndef QHTTPSERVER_RESPONSE_PRIVATE_HPP
#define QHTTPSERVER_RESPONSE_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////
#include "qhttpbase.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"

#include <QDateTime>
#include <QLocale>
#include <QTcpSocket>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHttpResponsePrivate : public HttpResponseBase,
        public HttpWriterBase<QHttpResponsePrivate>
{
    Q_DECLARE_PUBLIC(QHttpResponse)

public:
    explicit    QHttpResponsePrivate(QTcpSocket* sok, QHttpResponse* q)
        : HttpWriterBase(sok), q_ptr(q) {

        ikeepAlive                 = false;

        QObject::connect(isocket,      &QTcpSocket::disconnected, [this]() {
            ifinished   = true;
            q_func()->deleteLater();
        });

        QObject::connect(q_func(),     &QHttpResponse::done, [this](bool wasTheLastResponse) {
            if ( wasTheLastResponse )
                isocket->disconnectFromHost();
        });

        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpResponsePrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void        allBytesWritten() {
        emit q_func()->allBytesWritten();
    }

    void        ensureWritingHeaders();

    void        writeHeaders();

public:
    bool                 ikeepAlive;

protected:
    QHttpResponse*       q_ptr;

};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPSERVER_RESPONSE_PRIVATE_HPP
