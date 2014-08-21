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

class QHttpClientPrivate : public HttpParser<QHttpClientPrivate>
{
    Q_DECLARE_PUBLIC(QHttpClient)

public:
    explicit     QHttpClientPrivate(QHttpClient* q) : HttpParser(HTTP_RESPONSE), q_ptr(q) {
        QObject::connect(q_func(),    &QHttpClient::disconnected,    [this](){
            // if socket drops and http_parser can find messageComplete, calls it manually
            messageComplete(nullptr);
            isocket.release();

            if ( ilastRequest )
                ilastRequest->deleteLater();
            if ( ilastResponse )
                ilastResponse->deleteLater();
        });

        QHTTP_LINE_DEEPLOG
    }

    virtual     ~QHttpClientPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void         initializeSocket() {
        if ( isocket.isOpen() ) {
            if ( ikeepAlive ) // no need to reconnect. do nothing and simply return
                return;

            // close previous connection
            isocket.close();
        }

        ikeepAlive = false;

        // create a tcp connection
        if ( isocket.ibackendType == ETcpSocket ) {

            QTcpSocket* sok    =  new QTcpSocket(q_func());
            isocket.itcpSocket = sok;

            QObject::connect(sok,       &QTcpSocket::connected, [this](){
                onConnected();
            });
            QObject::connect(sok,       &QTcpSocket::readyRead, [this](){
                onReadyRead();
            });
            QObject::connect(sok,       &QTcpSocket::bytesWritten, [this](qint64){
                if ( isocket.itcpSocket->bytesToWrite() == 0  &&  ilastRequest )
                    emit ilastRequest->allBytesWritten();
            });
            QObject::connect(sok,       &QTcpSocket::disconnected,
                             q_func(),  &QHttpClient::disconnected);

        } else if ( isocket.ibackendType == ELocalSocket ) {

            QLocalSocket* sok    = new QLocalSocket(q_func());
            isocket.ilocalSocket = sok;

            QObject::connect(sok,       &QLocalSocket::connected, [this](){
                onConnected();
            });
            QObject::connect(sok,       &QLocalSocket::readyRead, [this](){
                onReadyRead();
            });
            QObject::connect(sok,       &QLocalSocket::bytesWritten, [this](qint64){
                if ( isocket.ilocalSocket->bytesToWrite() == 0  &&  ilastRequest )
                    emit ilastRequest->allBytesWritten();
            });
            QObject::connect(sok,       &QLocalSocket::disconnected,
                             q_func(),  &QHttpClient::disconnected);
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
    QHttpClient* const  q_ptr;

    QHttpRequest*       ilastRequest  = nullptr;
    QHttpResponse*      ilastResponse = nullptr;
    TRequstHandler      ireqHandler;
    TResponseHandler    irespHandler;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////

#endif // QHTTPCLIENT_PRIVATE_HPP
