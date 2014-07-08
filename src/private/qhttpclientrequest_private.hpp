#ifndef QHTTPCLIENT_REQUEST_PRIVATE_HPP
#define QHTTPCLIENT_REQUEST_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////
#include "qhttpbase.hpp"
#include "qhttpclientrequest.hpp"

#include <QTcpSocket>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
class QHttpRequestPrivate : public HttpRequestBase,
        public HttpWriterBase<QHttpRequestPrivate>
{
    Q_DECLARE_PUBLIC(QHttpRequest)

public:
    explicit    QHttpRequestPrivate(QTcpSocket* sok, QHttpRequest* q)
        : HttpWriterBase(sok), q_ptr(q) {
        ikeepAlive  = false;
        iversion    = "1.1";

        QObject::connect(isocket,      &QTcpSocket::disconnected, [this]() {
            ifinished   = true;
            q_func()->deleteLater();
        });

        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpRequestPrivate() {
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
    QHttpRequest* const  q_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPCLIENT_REQUEST_PRIVATE_HPP
