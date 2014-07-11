/** HTTP server class.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPSERVER_HPP
#define QHTTPSERVER_HPP

///////////////////////////////////////////////////////////////////////////////
#include "qhttpfwd.hpp"

#include <QTcpServer>
#include <QHostAddress>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////

/** The QHttpServer class is a fast, async (non-blocking) HTTP server. */
class QHttpServer : public QTcpServer
{
    Q_OBJECT

    Q_PROPERTY(quint32 timeOut READ timeOut WRITE setTimeOut)

public:
    /** construct a new HTTP Server. */
    explicit    QHttpServer(QObject *parent = nullptr);

    virtual    ~QHttpServer();

    /** starts the server on @c port listening on all interfaces. */
    bool        listen(const QHostAddress& address, quint16 port);

    /** @overload listen() */
    bool        listen(quint16 port) {
        return listen(QHostAddress::Any, port);
    }

    /** returns timeout value [mSec] for open connections (sockets).
     *  @sa setTimeOut(). */
    quint32     timeOut()const;

    /** set time-out for new open connections in miliseconds [mSec].
     * new incoming connections will be forcefully closed after this time out.
     *  a zero (0) value disables timer for new connections. */
    void        setTimeOut(quint32);

signals:
    /** emitted when a client makes a new request to the server if you do not override
     *  incomingConnection(QHttpConnection *connection);
     * @sa incommingConnection(). */
    void        newRequest(QHttpRequest *request, QHttpResponse *response);

protected:
    /** is called when server accepts a new connection.
     * you can override this function for using a thread-pool or ... some other reasons.
     *
     *  the default implementation just connects QHttpConnection::newRequest signal.
     * @note if you override this method, the signal won't be emitted by QHttpServer.
     * (perhaps, you do not need it anymore).
     *
     * @param connection New incoming connection. */
    virtual void incomingConnection(QHttpConnection* connection);

private:
    explicit    QHttpServer(QHttpServerPrivate&, QObject *parent);
    virtual void incomingConnection(qintptr handle);

    Q_DECLARE_PRIVATE(QHttpServer)
    Q_DISABLE_COPY(QHttpServer)
    QScopedPointer<QHttpServerPrivate>  d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // define QHTTPSERVER_HPP
