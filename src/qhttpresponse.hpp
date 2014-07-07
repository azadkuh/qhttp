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
#ifndef Q_HTTP_RESPONSE_HPP
#define Q_HTTP_RESPONSE_HPP

///////////////////////////////////////////////////////////////////////////////

#include "qhttpabstracts.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
/** The QHttpResponse class handles sending data back to the client as a response to a request.
 * The steps to respond correctly are
 * <ol>
 * <li>Call setHeader() to set headers [optional]</li>
 * <li>Call writeHead() with the HTTP status code</li>
 * <li>Call write() zero or more times for body data.</li>
 * <li>Call end() when the resonse can be sent back</li>
 * </ol>
 */
class QHTTPSERVER_API QHttpResponse : public QHttpAbstractOutput
{
    Q_OBJECT

public:
    virtual        ~QHttpResponse();

public:
    /** set the response HTTP status code. @sa TStatusCode.
     * default value is ESTATUS_BAD_REQUEST.
     * @sa write()
     */
    void            setStatusCode(TStatusCode code);

public: // QHttpAbstractOutput methods:
    /** @see QHttpAbstractOutput::setVersion(). */
    void            setVersion(const QString& versionString);

    /** @see QHttpAbstractOutput::addHeader(). */
    void            addHeader(const QByteArray& field, const QByteArray& value);

    /** @see QHttpAbstractOutput::headers(). */
    THeaderHash&    headers();

    /** @see QHttpAbstractOutput::write(). */
    void            write(const QByteArray &data);

    /** @see QHttpAbstractOutput::end(). */
    void            end(const QByteArray &data = QByteArray());


protected:
    explicit        QHttpResponse(QTcpSocket*);
    explicit        QHttpResponse(QHttpResponsePrivate&, QTcpSocket*);
    friend class    QHttpConnectionPrivate;

    Q_DECLARE_PRIVATE(QHttpResponse)
    QScopedPointer<QHttpResponsePrivate> d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // define Q_HTTP_RESPONSE_HPP
