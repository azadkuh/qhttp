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
    Q_DECLARE_PUBLIC(QHttpRequest)

public:
    explicit    QHttpRequestPrivate(QTcpSocket* sok, QHttpRequest* q) : q_ptr(q) {
        isuccessful = false;
        iremotePort = 0;
        isocket     = sok;

        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpRequestPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void        initialize() {
        QObject::connect(isocket, &QTcpSocket::disconnected, [this](){
            q_ptr->deleteLater();
        });
    }

public:
    QString     iremoteAddress;
    quint16     iremotePort;
    bool        isuccessful;

protected:
    QTcpSocket*             isocket;
    QHttpRequest* const     q_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPSERVER_REQUEST_PRIVATE_HPP
