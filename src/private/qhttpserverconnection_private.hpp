/** private imeplementation.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPSERVER_CONNECTION_PRIVATE_HPP
#define QHTTPSERVER_CONNECTION_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpserverconnection.hpp"

#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include "private/qhttpserverrequest_private.hpp"
#include "private/qhttpserverresponse_private.hpp"

#include <QBasicTimer>
#include <QFile>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHttpConnectionPrivate  : public HttpParserBase<QHttpConnectionPrivate>
{
protected:
    Q_DECLARE_PUBLIC(QHttpConnection)
    QHttpConnection* const q_ptr;

public:
    QByteArray              itempUrl;

    // Since there can only be one request at any time even with pipelining.
    QHttpRequest*           irequest  = nullptr;     ///< latest request
    QHttpResponse*          iresponse = nullptr;     ///< latest response

    QBasicTimer             itimer;

    TServerHandler          ihandler = nullptr;

public:
    explicit     QHttpConnectionPrivate(QHttpConnection* q)
        : HttpParserBase(HTTP_REQUEST), q_ptr(q) {

        QObject::connect(q_func(), &QHttpConnection::disconnected, [this](){
            // if socket drops and http_parser can find messageComplete, calls it manually
            messageComplete(nullptr);

            irequest    = nullptr;
            iresponse   = nullptr;

            if ( itcpSocket )
                itcpSocket->deleteLater();

            if ( ilocalSocket )
                ilocalSocket->deleteLater();

            q_func()->deleteLater();
        });

        QHTTP_LINE_DEEPLOG
    }

    virtual     ~QHttpConnectionPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void         initialize(qintptr sokDesc, TBackend bend) {
        Q_ASSERT(itcpSocket == nullptr    &&    ilocalSocket == nullptr);

        ibackendType = bend;
        if        ( ibackendType == ETcpSocket ) {
            itcpSocket = new QTcpSocket( q_func() );
            itcpSocket->setSocketDescriptor(sokDesc);

            QObject::connect(itcpSocket, &QTcpSocket::readyRead, [this](){
                while ( itcpSocket->bytesAvailable() ) {
                    char buffer[4096] = {0};
                    size_t readLength = itcpSocket->read(buffer, 4095);

                    parse(buffer, readLength);
                }
            });

            QObject::connect(itcpSocket, &QTcpSocket::disconnected,
                             q_func(),   &QHttpConnection::disconnected,
                             Qt::QueuedConnection);

        } else if ( ibackendType == ELocalSocket ) {
            ilocalSocket = new QLocalSocket( q_func() );
            ilocalSocket->setSocketDescriptor(sokDesc);

            QObject::connect(ilocalSocket, &QTcpSocket::readyRead, [this](){
                while ( ilocalSocket->bytesAvailable() ) {
                    char buffer[4096] = {0};
                    size_t readLength = ilocalSocket->read(buffer, 4095);

                    parse(buffer, readLength);
                }
            });

            QObject::connect(ilocalSocket, &QLocalSocket::disconnected,
                             q_func(),      &QHttpConnection::disconnected,
                             Qt::QueuedConnection);
        }

    }

public:
    int          messageBegin(http_parser* parser);
    int          url(http_parser* parser, const char* at, size_t length);
    int          status(http_parser*, const char*, size_t) {
        return 0;   // not used in parsing incoming request.
    }
    int          headerField(http_parser* parser, const char* at, size_t length);
    int          headerValue(http_parser* parser, const char* at, size_t length);
    int          headersComplete(http_parser* parser);
    int          body(http_parser* parser, const char* at, size_t length);
    int          messageComplete(http_parser* parser);

#ifdef USE_CUSTOM_URL_CREATOR
public:
    static QUrl  createUrl(const char *urlData, const http_parser_url &urlInfo);
#endif // USE_CUSTOM_URL_CREATOR

};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPSERVER_CONNECTION_PRIVATE_HPP
