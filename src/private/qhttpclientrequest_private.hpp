/** private imeplementation.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPCLIENT_REQUEST_PRIVATE_HPP
#define QHTTPCLIENT_REQUEST_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////
#include "qhttpbase.hpp"
#include "qhttpclient.hpp"
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
    explicit    QHttpRequestPrivate(QHttpClient* cli, QHttpRequest* q)
        : HttpWriterBase(), q_ptr(q), iclient(cli) {
        iversion    = "1.1";

        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpRequestPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void        initialize() {
        if ( iclient->backendType() == ETcpSocket )
            itcpSocket   = iclient->tcpSocket();
        else if ( iclient->backendType() == ELocalSocket )
            ilocalSocket = iclient->localSocket();

        HttpWriterBase::initialize();

        QObject::connect(iclient,      &QHttpClient::disconnected, [this]() {
            ifinished   = true;
            q_ptr->deleteLater();
        });
    }

    void        allBytesWritten() {
        emit q_func()->allBytesWritten();
    }

    void        ensureWritingHeaders();

    void        writeHeaders();

public:
    bool                 ikeepAlive = false;

protected:
    QHttpRequest* const  q_ptr;
    QHttpClient* const   iclient;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPCLIENT_REQUEST_PRIVATE_HPP
