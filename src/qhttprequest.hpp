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

#include "qhttpserverapi.hpp"
#include "qhttpserverfwd.hpp"

#include <QObject>
#include <QMetaEnum>
#include <QMetaType>
#include <QUrl>
///////////////////////////////////////////////////////////////////////////////

/** The QHttpRequest class represents the header and body data sent by the client.
 * it comes in via the data() signal. As a consequence the application's request
 *  callback should ensure that it connects to the data() signal before control
 *  returns back to the event loop. Otherwise there is a risk of some data never
 *  being received by the application.
 *
 * The class is <b>read-only</b>.
 */
class QHTTPSERVER_API QHttpRequest : public QObject
{
    Q_OBJECT

    Q_PROPERTY(THeaderHash  headers         READ headers)
    Q_PROPERTY(QString      remoteAddress   READ remoteAddress)
    Q_PROPERTY(quint16      remotePort      READ remotePort)
    Q_PROPERTY(QString      method          READ method)
    Q_PROPERTY(QUrl         url             READ url)
    Q_PROPERTY(QString      path            READ path)
    Q_PROPERTY(QString      httpVersion     READ httpVersion)

    Q_ENUMS(HttpMethod)

public:
    /** Request method enumeration.
     * @note Taken from http_parser.h
     * -- make sure to keep synced */
    enum HttpMethod {
        HTTP_DELETE = 0,
        HTTP_GET,
        HTTP_HEAD,
        HTTP_POST,
        HTTP_PUT,
        // pathological
        HTTP_CONNECT,
        HTTP_OPTIONS,
        HTTP_TRACE,
        // webdav
        HTTP_COPY,
        HTTP_LOCK,
        HTTP_MKCOL,
        HTTP_MOVE,
        HTTP_PROPFIND,
        HTTP_PROPPATCH,
        HTTP_SEARCH,
        HTTP_UNLOCK,
        // subversion
        HTTP_REPORT,
        HTTP_MKACTIVITY,
        HTTP_CHECKOUT,
        HTTP_MERGE,
        // upnp
        HTTP_MSEARCH,
        HTTP_NOTIFY,
        HTTP_SUBSCRIBE,
        HTTP_UNSUBSCRIBE,
        // RFC-5789
        HTTP_PATCH,
        HTTP_PURGE
    };

public:
    virtual ~QHttpRequest();

    /** The method used for the request. */
    HttpMethod          method() const ;

    /** Returns the method string for the request.
     * @note This will plainly transform the enum into a string HTTP_GET -> "HTTP_GET". */
    const QString       methodString() const;

    /** The complete URL for the request.
     * This includes the path and query string. @sa path(). */
    const QUrl&         url() const;

    /** The path portion of the query URL.
     * @sa url(). */
    const QString       path() const;

    /** The HTTP version of the request.
     * @return A string in the form of "x.x" */
    const QString&      httpVersion() const;

    /** Return all the headers sent by the client.
     * This returns a reference. If you want to store headers
     *  somewhere else, where the request may be deleted,
     *  make sure you store them as a copy.
     * @note All header names are <b>lowercase</b> . */
    const THeaderHash&  headers() const;

    /** Get the value of a header.
     * Headers are stored as lowercase so the input @c field will be lowercased.
     * @param field Name of the header field
     * @return Value of the header or empty string if not found. */
    QString             header(const QByteArray &field);

    /** IP Address of the client in dotted decimal format. */
    const QString&      remoteAddress() const;

    /** Outbound connection port for the client. */
    quint16             remotePort() const;

    /** If this request was successfully received.
     * Set before end() has been emitted, stating whether
     *  the message was properly received. This is false
     *  until the receiving the full request has completed. */
    bool                successful() const;

signals:
    /** Emitted when new body data has been received.
     * @note This may be emitted zero or more times
     *  depending on the request type. @param data Received data. */
    void                data(const QByteArray &data);

    /** Emitted when the request has been fully received.
     * @note The no more data() signals will be emitted after this. */
    void                end();

public:
    /** Set the value of a HTTP header.
     * @note You must call this with all your custom headers
     *  before calling writeHead(), write() or end(). */
    void                setHeader(const QByteArray& field, const QByteArray& value);

private:
    class               Private;
    Private*            pimp;


    static QString      MethodToString(QHttpRequest::HttpMethod method);

    explicit            QHttpRequest(QObject *connection);

    friend class        QHttpConnection;
    friend class        QHttpClient;
};

///////////////////////////////////////////////////////////////////////////////

#endif // define Q_HTTP_REQUEST_HPP
