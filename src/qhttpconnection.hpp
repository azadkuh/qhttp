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

#include <QObject>

///////////////////////////////////////////////////////////////////////////////

/// @cond nodoc

class QHTTPSERVER_API QHttpConnection : public QObject
{
    Q_OBJECT

public:
    virtual ~QHttpConnection();

    void    write(const QByteArray &data);
    void    flush();

signals:
    void    newRequest(QHttpRequest *, QHttpResponse *);
    void    allBytesWritten();

private slots:
    void    parseRequest();
    void    responseDone();
    void    socketDisconnected();
    void    updateWriteCount(qint64);

private:
    friend class QHttpServer;
    explicit QHttpConnection(qintptr handle, QObject *parent);

    QTcpSocket*             m_socket;
    http_parser*            m_parser;
    http_parser_settings*   m_parserSettings;

    // Since there can only be one request at any time even with pipelining.
    QHttpRequest*           m_request;

    QByteArray              m_currentUrl;
    // The ones we are reading in from the parser
    THeaderHash             m_currentHeaders;
    QByteArray              m_currentHeaderField;
    QByteArray              m_currentHeaderValue;

    // Keep track of transmit buffer status
    qint64                  m_transmitLen;
    qint64                  m_transmitPos;

    class                   Private;
};

/// @endcond

///////////////////////////////////////////////////////////////////////////////
#endif // #define Q_HTTP_CONNECTION_HPP
