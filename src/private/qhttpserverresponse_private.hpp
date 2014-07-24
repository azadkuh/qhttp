/** private imeplementation.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

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
    explicit    QHttpResponsePrivate(QHttpConnection* conn, QHttpResponse* q)
        : HttpWriterBase(), q_ptr(q), iconnection(conn) {
        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpResponsePrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void        initialize() {
        if ( iconnection->backendType() == ETcpSocket )
            itcpSocket = iconnection->tcpSocket();
        else if ( iconnection->backendType() == ELocalSocket )
            ilocalSocket = iconnection->localSocket();

        HttpWriterBase::initialize();

        QObject::connect(q_func(),     &QHttpResponse::done, [this](bool wasTheLastResponse) {
            if ( wasTheLastResponse )
                iconnection->killConnection();
        });

        QObject::connect(iconnection,  &QHttpConnection::disconnected, [this]() {
            ifinished   = true;
            q_func()->deleteLater();
        });
    }

    void        allBytesWritten() {
        emit q_func()->allBytesWritten();
    }

    void        ensureWritingHeaders();

    void        writeHeaders();

public:
    bool        ikeepAlive = false;

protected:
    QHttpResponse* const    q_ptr;
    QHttpConnection* const  iconnection;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPSERVER_RESPONSE_PRIVATE_HPP
