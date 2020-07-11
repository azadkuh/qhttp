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
#include "httpparser.hxx"
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
class QHttpConnectionPrivate  :
    public details::HttpRequestParser<QHttpConnectionPrivate>
{
    Q_DECLARE_PUBLIC(QHttpConnection)

public:
    explicit QHttpConnectionPrivate(QHttpConnection* q, TBackend backendType) : q_ptr(q) {
        if(backendType == ETcpSocket)
            isocket.reset(new details::QHttpTcpSocket);
        else
            isocket.reset(new details::QHttpLocalSocket);

        QObject::connect(
            q_func(), &QHttpConnection::disconnected,
            [this](){ release(); }
        );

        QHTTP_LINE_DEEPLOG
    }

    virtual ~QHttpConnectionPrivate();

    void createSocket(qintptr sokDesc) {
        isocket->init (
              q_func(),
              sokDesc,
              [this](){ onReadyRead();  },
              [this](){ onWriteReady(); },
              [this](){ emit q_func()->disconnected (); }
        );
    }

    void release() {
        // if socket drops and http_parser can not call
        // messageComplete, dispatch the ilastRequest
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

        q_func()->deleteLater();
    }

public:
    void onReadyRead() {
        while ( isocket->bytesAvailable() > 0 ) {
            char buffer[4097] = {0};
            size_t readLength = static_cast<size_t>(isocket->readRaw(buffer, 4096));

            parse(buffer, readLength);
            if (iparser.http_errno != 0) {
                release(); // release the socket if parsing failed
                return;
            }
        }
    }

    void onWriteReady() {
        auto bytesWritten = isocket->bytesToWrite();
        if ( bytesWritten == 0  &&  ilastResponse)
            emit ilastResponse->allBytesWritten();
    }

    void finalizeConnection() {
        if ( ilastRequest == nullptr )
            return;

        ilastRequest->pPrivate->finalizeSending([this]{
            emit ilastRequest->end();
        });
    }

public:
    int  messageBegin(http_parser* parser);
    int  url(http_parser* parser, const char* at, int length);
    int  status(http_parser*, const char*, size_t) {
        return 0;   // not used in parsing incoming request.
    }
    int  headerField(http_parser* parser, const char* at, int length);
    int  headerValue(http_parser* parser, const char* at, int length);
    int  headersComplete(http_parser* parser);
    int  body(http_parser* parser, const char* at, int length);
    int  messageComplete(http_parser* parser);

protected:
    QHttpConnection* const q_ptr;

    QByteArray             itempUrl;

    // Since there can only be one request/response pair per connection at any
    // time even with pipelining.
    QHttpRequest*          ilastRequest  = nullptr;
    QHttpResponse*         ilastResponse = nullptr;

    TServerHandler         ihandler      = nullptr;

};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPSERVER_CONNECTION_PRIVATE_HPP
