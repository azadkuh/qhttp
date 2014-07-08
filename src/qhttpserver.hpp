/*
 * Copyright 2011-2014 Nikhil Marathe <nsm.nikhil@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef Q_HTTP_SERVER_HPP
#define Q_HTTP_SERVER_HPP

#define QHTTPSERVER_VERSION_MAJOR 1
#define QHTTPSERVER_VERSION_MINOR 0
#define QHTTPSERVER_VERSION_PATCH 0

///////////////////////////////////////////////////////////////////////////////
#include "qhttpserverapi.hpp"
#include "qhttpserverfwd.hpp"

#include <QTcpServer>
#include <QHostAddress>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////

/** The QHttpServer class forms the basis of this project.
 * It is a fast, non-blocking HTTP server.
 *  These are the steps to create a server, handle and respond to requests:
 * <ol>
 *     <li>Create an instance of QHttpServer.</li>
 *     <li>Connect a slot to the newRequest() signal.</li>
 *     <li>Create a QCoreApplication to drive the server event loop.</li>
 *     <li>Respond to clients by writing out to the QHttpResponse object.</li>
 * </ol>
 */
class QHTTPSERVER_API QHttpServer : public QTcpServer
{
    Q_OBJECT

    Q_PROPERTY(quint32 timeOut READ timeOut WRITE setTimeOut)

public:
    /** construct a new HTTP Server. */
    explicit    QHttpServer(QObject *parent = nullptr);

    virtual    ~QHttpServer();

    /** starts the server on @c port listening on all interfaces. */
    bool        listen(const QHostAddress& address, quint16 port);

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
#endif // define Q_HTTP_SERVER_HPP
