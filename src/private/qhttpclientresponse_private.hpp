#ifndef QHTTP_CLIENT_RESPONSE_PRIVATE_HPP
#define QHTTP_CLIENT_RESPONSE_PRIVATE_HPP
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
    explicit QHttpResponsePrivate(QTcpSocket* sok, QHttpResponse* q) : q_ptr(q) {
        Q_ASSERT(q_ptr);
        isuccessful = false;

        QObject::connect(sok, &QTcpSocket::disconnected, [this](){
            q_ptr->deleteLater();
        });

        QHTTP_LINE_DEEPLOG
    }

    virtual ~QHttpResponsePrivate() {
        QHTTP_LINE_DEEPLOG
    }

public:
    bool                    isuccessful;
    QString                 icustomeStatusMessage;

protected:
    QHttpResponse* const    q_ptr;
    Q_DECLARE_PUBLIC(QHttpResponse)
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTP_CLIENT_RESPONSE_PRIVATE_HPP
