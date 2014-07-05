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

#ifndef Q_HTTP_REQUEST_HPP
#define Q_HTTP_REQUEST_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpabstracts.hpp"

#include <QUrl>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
/** The QHttpRequest class represents the header and body data sent by the client.
 * it comes in via the data() signal. As a consequence the application's request
 *  callback should ensure that it connects to the data() signal before control
 *  returns back to the event loop. Otherwise there is a risk of some data never
 *  being received by the application.
 *
 * The class is <b>read-only</b>.
 */
class QHTTPSERVER_API QHttpRequest : public QHttpAbstractInput
{
    Q_OBJECT

public:
    virtual            ~QHttpRequest();

    /** @see QHttpAbstractInput::headers(). */
    const THeaderHash&  headers() const;

    /** @see QHttpAbstractInput::httpVersion(). */
    const QString&      httpVersion() const;

    /** @see QHttpAbstractInput::isSuccessful(). */
    bool                isSuccessful() const;


    /** The method used for the request. */
    THttpMethod         method() const ;

    /** Returns the method string for the request.
     * @note This will plainly transform the enum into a string HTTP_GET -> "HTTP_GET". */
    const QString       methodString() const;

    /** The complete URL for the request.
     * This includes the path and query string. @sa path(). */
    const QUrl&         url() const;

    /** IP Address of the client in dotted decimal format. */
    const QString&      remoteAddress() const;

    /** Outbound connection port for the client. */
    quint16             remotePort() const;


protected:
    explicit            QHttpRequest(QTcpSocket*);
    explicit            QHttpRequest(QHttpRequestPrivate&, QTcpSocket*);
    friend class        QHttpConnectionPrivate;

    Q_DECLARE_PRIVATE(QHttpRequest)
    QScopedPointer<QHttpRequestPrivate> d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // define Q_HTTP_REQUEST_HPP
