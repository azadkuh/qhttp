#ifndef QHTTPCLIENT_HPP
#define QHTTPCLIENT_HPP

///////////////////////////////////////////////////////////////////////////////
#include "qhttpfwd.hpp"

#include <QObject>
#include <QUrl>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////

/** a simple HTTP client class which sends a request to an HTTP server and parses the
 *  corresponding response.
 * This class internally handles the memory management and life cycle of QHttpRequest and
 *  QHttpResponse instances. you do not have to manually delete or keep their pointers.
 * in fact the QHttpRequest and QHttpResponse object will be deleted when the internal socket
 *  disconnects.
 */
class QHttpClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 timeOut READ timeOut WRITE setTimeOut)

public:
    explicit    QHttpClient(QObject *parent = nullptr);

    virtual    ~QHttpClient();

    /** connects to a server.
     *  if the connection has been made, creates and emits a QHttpRequest instance
     *   by @sa connected(QHttpRequest*)
     * @param method an HTTP method, ex: GET, POST, ...
     * @param url specifies server's address, port and optional path and query strings.
     */
    void        request(THttpMethod method, QUrl url);

    /** checks if the connetion to the server is open. */
    bool        isOpen() const;

    /** frocefully disconnects from server and closes the connection. */
    void        close();


    /** returns time-out value [mSec] for open connections (sockets).
     *  @sa setTimeOut(). */
    quint32     timeOut()const;

    /** set time-out for new open connections in miliseconds [mSec].
     * each connection will be forcefully closed after this timeout.
     *  a zero (0) value disables timer for new connections. */
    void        setTimeOut(quint32);

signals:
    /** emitted when a new HTTP connection to the server is established.
     * @param req the request instance for assinging the request headers and body.
     * @sa request()
     * @sa QHttpRequest
     */
    void        connected(QHttpRequest* req);

    /** emitted when a new response is received from the server.
     * @param res the instance for reading incoming response.
     * @sa QHttpResponse
     */
    void        newResponse(QHttpResponse* res);

    /** emitted when the connection has been dropped or disconnected. */
    void        disconnected();

protected:
    explicit    QHttpClient(QHttpClientPrivate&, QObject*);

    void        timerEvent(QTimerEvent*);

    Q_DECLARE_PRIVATE(QHttpClient)
    Q_DISABLE_COPY(QHttpClient)
    QScopedPointer<QHttpClientPrivate>  d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // define QHTTPCLIENT_HPP
