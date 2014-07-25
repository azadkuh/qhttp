/** private imeplementation.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPCLIENT_PRIVATE_HPP
#define QHTTPCLIENT_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpclient.hpp"
#include "qhttpclientrequest_private.hpp"
#include "qhttpclientresponse_private.hpp"

#include <QPointer>
#include <QBasicTimer>
#include <QFile>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////

class QHttpClientPrivate : public HttpParserBase<QHttpClientPrivate>
{
    Q_DECLARE_PUBLIC(QHttpClient)

public:
    THttpMethod     ilastMethod;
    QUrl            ilastUrl;

    quint32         itimeOut = 0;
    QBasicTimer     itimer;

public:
    explicit     QHttpClientPrivate(QHttpClient* q) : HttpParserBase(HTTP_RESPONSE), q_ptr(q) {
        QObject::connect(q, &QHttpClient::disconnected, [this](){
            ilastResponse = nullptr;
            q_func()->deleteLater();
        });
        QHTTP_LINE_DEEPLOG
    }

    virtual     ~QHttpClientPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void         initializeSocket() {
        if ( q_func()->backendType() == ETcpSocket ) {
            itcpSocket   = new QTcpSocket(q_func());

            QObject::connect(itcpSocket,  &QTcpSocket::connected, [this](){
                onConnected();
            });
            QObject::connect(itcpSocket,  &QTcpSocket::readyRead, [this](){
                onReadyRead();
            });
            QObject::connect(itcpSocket,  &QTcpSocket::disconnected,
                             q_func(),    &QHttpClient::disconnected);

        } else if ( q_func()->backendType() == ELocalSocket ) {
            ilocalSocket = new QLocalSocket(q_func());

            QObject::connect(ilocalSocket,  &QLocalSocket::connected, [this](){
                onConnected();
            });
            QObject::connect(ilocalSocket,  &QLocalSocket::readyRead, [this](){
                onReadyRead();
            });
            QObject::connect(ilocalSocket,  &QLocalSocket::disconnected,
                             q_func(),      &QHttpClient::disconnected);
        }
    }

public:
    int          messageBegin(http_parser* parser);
    int          url(http_parser*, const char*, size_t) {
        return 0; // not used in parsing incoming respone.
    }
    int          status(http_parser* parser, const char* at, size_t length) ;
    int          headerField(http_parser* parser, const char* at, size_t length);
    int          headerValue(http_parser* parser, const char* at, size_t length);
    int          headersComplete(http_parser* parser);
    int          body(http_parser* parser, const char* at, size_t length);
    int          messageComplete(http_parser* parser);

protected:
    void         onConnected();
    void         onReadyRead();

protected:
    QHttpClient* const      q_ptr;

    QHttpResponse*          ilastResponse = nullptr;
    TRequstHandler          ireqHandler;
    TResponseHandler        irespHandler;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////

#endif // QHTTPCLIENT_PRIVATE_HPP
