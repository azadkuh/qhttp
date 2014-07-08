#ifndef QHTTPSERVER_REQUEST_PRIVATE_HPP
#define QHTTPSERVER_REQUEST_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpbase.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverconnection.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHttpRequestPrivate : public HttpRequestBase
{
public:
    explicit QHttpRequestPrivate(QTcpSocket* sok, QHttpRequest* q) : q_ptr(q) {
        Q_ASSERT(q_ptr);

        isuccessful = false;
        iremotePort = 0;

        QObject::connect(sok, &QTcpSocket::disconnected, [this](){
            q_ptr->deleteLater();
        });

        QHTTP_LINE_DEEPLOG
    }

    virtual ~QHttpRequestPrivate() {
        QHTTP_LINE_DEEPLOG
    }

public:
    QString                  iremoteAddress;
    quint16                  iremotePort;
    bool                     isuccessful;

protected:
    QHttpRequest* const     q_ptr;
    Q_DECLARE_PUBLIC(QHttpRequest)
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPSERVER_REQUEST_PRIVATE_HPP
