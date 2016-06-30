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
#include "httpwriter.hxx"
#include "qhttp/qhttpserverresponse.hpp"
#include "qhttp/qhttpserver.hpp"
#include "qhttp/qhttpserverconnection.hpp"

#include <QDateTime>
#include <QLocale>
#include <QTcpSocket>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHttpResponsePrivate :
    public details::HttpWriter<details::HttpResponseBase, QHttpResponsePrivate>
{
    Q_DECLARE_PUBLIC(QHttpResponse)

public:
    explicit    QHttpResponsePrivate(QHttpConnection* conn, QHttpResponse* q)
        : q_ptr(q), iconnection(conn) { }

    virtual    ~QHttpResponsePrivate() = default;

    void        initialize() {
        isocket.ibackendType = iconnection->backendType();
        isocket.ilocalSocket = iconnection->localSocket();
        isocket.itcpSocket   = iconnection->tcpSocket();

        QObject::connect(iconnection,  &QHttpConnection::disconnected,
                         q_func(),     &QHttpResponse::deleteLater);
    }

    QByteArray  makeTitle();

    void        prepareHeadersToWrite();

protected:
    QHttpResponse* const    q_ptr;
    QHttpConnection* const  iconnection;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPSERVER_RESPONSE_PRIVATE_HPP
