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
#include "httpparser.hxx"
#include "qhttpclientrequest_private.hpp"
#include "qhttpclientresponse_private.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////

class QHttpClientPrivate :
    public details::HttpResponseParser<QHttpClientPrivate>
{
    Q_DECLARE_PUBLIC(QHttpClient)

public:
    explicit QHttpClientPrivate(QHttpClient* q, TBackend backendType) : q_ptr(q) {
        if(backendType == ETcpSocket)
            isocket.reset(new details::QHttpTcpSocket);
        else
            isocket.reset(new details::QHttpLocalSocket);

        QObject::connect(
            q_func(), &QHttpClient::disconnected,
            [this](){ release(); }
        );

        QHTTP_LINE_DEEPLOG
    }

    virtual ~QHttpClientPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void release() {
        // if socket drops and http_parser can not call messageComplete,
        // dispatch the ilastResponse
        finalizeConnection();

        isocket->disconnectAllQtConnections();
        isocket->release();

        if ( ilastRequest ) {
            ilastRequest->deleteLater();
            ilastRequest  = nullptr;
        }

        if ( ilastResponse ) {
            ilastResponse->deleteLater();
            ilastResponse = nullptr;
        }

        // must be called! or the later http_parser_execute() may fail
        http_parser_init(&iparser, HTTP_RESPONSE);
    }

    void initializeSocket() {
        if ( isocket->isOpen() ) {
            // no need to reconnect. do nothing and simply return
            if ( ikeepAlive )
                return;

            // close previous connection now!
            // instead being called by emitted disconnected signal
            release();
        }

        ikeepAlive = false;

        isocket->init (
                    [this](){ onConnected(); },
                    [this](){ onReadyRead(); },
                    [this](){ onWriteReady(); },
                    [this](){ emit q_func()->disconnected(); }
        );
    }

public:
    int  messageBegin(http_parser* parser);
    int  url(http_parser*, const char*, size_t) {
        return 0; // not used in parsing incoming respone.
    }
    int  status(http_parser* parser, const char* at, size_t length) ;
    int  headerField(http_parser* parser, const char* at, size_t length);
    int  headerValue(http_parser* parser, const char* at, size_t length);
    int  headersComplete(http_parser* parser);
    int  body(http_parser* parser, const char* at, size_t length);
    int  messageComplete(http_parser* parser);

protected:
    void onConnected() {
        iconnectingTimer.stop();

        if ( itimeOut > 0 )
            itimer.start(itimeOut, Qt::CoarseTimer, q_func());

        if ( ireqHandler )
            ireqHandler(ilastRequest);
        else
            q_func()->onRequestReady(ilastRequest);
    }

    void onReadyRead() {
        while ( isocket->bytesAvailable() > 0 ) {
            char buffer[4097] = {0};
            size_t readLength = (size_t) isocket->readRaw(buffer, 4096);

            parse(buffer, readLength);
        }
    }

    void onWriteReady(){
        if ( isocket->bytesToWrite() == 0  &&  ilastRequest )
            emit ilastRequest->allBytesWritten();
    }

    void finalizeConnection() {
        if ( ilastResponse == nullptr )
            return;

        ilastResponse->d_func()->finalizeSending([this]{
            emit ilastResponse->end();
        });
    }

protected:
    QHttpClient* const  q_ptr;

    QHttpRequest*       ilastRequest  = nullptr;
    QHttpResponse*      ilastResponse = nullptr;
    TRequstHandler      ireqHandler;
    TResponseHandler    irespHandler;

    QBasicTimer         iconnectingTimer;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////

#endif // QHTTPCLIENT_PRIVATE_HPP
