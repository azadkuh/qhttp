#ifndef QHTTPCLIENT_RESPONSE_PRIVATE_HPP
#define QHTTPCLIENT_RESPONSE_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpbase.hpp"
#include "qhttpclientresponse.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
class QHttpResponsePrivate : public HttpResponseBase
{
public:
    explicit    QHttpResponsePrivate(QTcpSocket* sok, QHttpResponse* q)
        : isocket(sok), q_ptr(q) {
        isuccessful = false;

        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpResponsePrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void       initialize() {
        QObject::connect(isocket, &QTcpSocket::disconnected, [this](){
            q_ptr->deleteLater();
        });
    }

public:
    bool                    isuccessful;
    QString                 icustomeStatusMessage;

protected:
    QTcpSocket*             isocket;
    QHttpResponse* const    q_ptr;
    Q_DECLARE_PUBLIC(QHttpResponse)
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPCLIENT_RESPONSE_PRIVATE_HPP
