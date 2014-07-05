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

#ifndef Q_HTTP_CONNECTION_HPP
#define Q_HTTP_CONNECTION_HPP
///////////////////////////////////////////////////////////////////////////////
#include "qhttpserverapi.hpp"
#include "qhttpserverfwd.hpp"

#include <QTcpSocket>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHTTPSERVER_API QHttpConnection : public QTcpSocket
{
    Q_OBJECT

public:
    virtual        ~QHttpConnection();

    /** set an optional timer event to close the connection. */
    void            setTimeOut(quint32 miliSeconds);

    /** forcefully kill (close) a connection. */
    void            killConnection();

    QHttpRequest*   latestRequest() const;
    QHttpResponse*  latestResponse() const;

signals:
    void            newRequest(QHttpRequest *, QHttpResponse *);

protected:
    explicit        QHttpConnection(QObject *parent);
    explicit        QHttpConnection(QHttpConnectionPrivate&, QObject *);
    void            timerEvent(QTimerEvent*);

    Q_DISABLE_COPY(QHttpConnection)
    Q_DECLARE_PRIVATE(QHttpConnection)
    QScopedPointer<QHttpConnectionPrivate>    d_ptr;

    friend class    QHttpServer;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // #define Q_HTTP_CONNECTION_HPP
