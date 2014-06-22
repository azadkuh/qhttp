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
#include "private/qhttpresponse_private.hpp"
///////////////////////////////////////////////////////////////////////////////
    // TODO: parent child relation
QHttpResponse::QHttpResponse(QHttpConnection *connection)
    : QObject(connection) , pimp(0) {
    pimp    = new Private(connection);
}

QHttpResponse::~QHttpResponse() {
}

void
QHttpResponse::setHeader(const QByteArray &field, const QByteArray &value) {
    if ( !pimp->m_finished )
        pimp->m_headers[field] = value;
    else
        qWarning() << "QHttpResponse::setHeader() Cannot set headers after response has finished.";
}

void
QHttpResponse::writeHead(int status) {
    if ( pimp->m_finished ) {
        qWarning()
            << "QHttpResponse::writeHead() Cannot write headers after response has finished.";
        return;
    }

    if ( pimp->m_headerWritten ) {
        qWarning() << "QHttpResponse::writeHead() Already called once for this response.";
        return;
    }

    pimp->m_connection->write(QString("HTTP/1.1 %1 %2\r\n")
                              .arg(status)
                              .arg(QHttpServer::statusCodes()[status]).toLatin1()
                              );
    pimp->writeHeaders();
    pimp->m_connection->write("\r\n");

    pimp->m_headerWritten = true;
}

void
QHttpResponse::writeHead(StatusCode statusCode) {
    writeHead(static_cast<int>(statusCode));
}

void
QHttpResponse::write(const QByteArray &data) {
    if ( pimp->m_finished ) {
        qWarning() << "QHttpResponse::write() Cannot write body after response has finished.";
        return;
    }

    if ( !pimp->m_headerWritten ) {
        qWarning() << "QHttpResponse::write() You must call writeHead() before writing body data.";
        return;
    }

    pimp->m_connection->write(data);
}

void
QHttpResponse::end(const QByteArray &data) {
    if ( pimp->m_finished ) {
        qWarning() << "QHttpResponse::end() Cannot write end after response has finished.";
        return;
    }

    if ( data.size() > 0 )
        write(data);

    pimp->m_finished = true;

    emit done();

    /// @todo End connection and delete ourselves. Is this a still valid note?
    deleteLater();
}

void
QHttpResponse::connectionClosed() {
    pimp->m_finished = true;
    deleteLater();
}


///////////////////////////////////////////////////////////////////////////////
void
QHttpResponse::Private::writeHeader(const char *field, const QByteArray &value) {
    if ( !m_finished ) {
        m_connection->write(field);
        m_connection->write(": ");
        m_connection->write(value.toLower());
        m_connection->write("\r\n");
    } else
        qWarning()
            << "QHttpResponse::writeHeader() Cannot write headers after response has finished.";
}

void
QHttpResponse::Private::writeHeaders() {
    if ( m_finished )
        return;

    for ( THeaderHash::const_iterator cit = m_headers.begin();
          cit != m_headers.end(); cit++ ) {
        const QByteArray& name  = cit.key();
        const QByteArray& value = cit.value();

        if ( qstrnicmp("connection", name.constData(), name.length()) == 0 ) {
            m_sentConnectionHeader = true;
            if ( qstrnicmp("close", value.constData(), value.length()) == 0 )
                m_last = true;
            else
                m_keepAlive = true;

        } else if ( qstrnicmp("transfer-encoding", name.constData(), name.length()) == 0 ) {
            m_sentTransferEncodingHeader = true;
            if ( qstrnicmp("chunked", value.constData(), value.length()) == 0 )
                m_useChunkedEncoding = true;

        } else if ( qstrnicmp("content-length", name.constData(), name.length()) == 0 ) {
            m_sentContentLengthHeader = true;

        } else if ( qstrnicmp("date", name.constData(), name.length()) == 0 ) {
            m_sentDate = true;
        }

        writeHeader(name.constData(), value);
    }

    if ( !m_sentConnectionHeader ) {
        if ( m_keepAlive && ( m_sentContentLengthHeader || m_useChunkedEncoding ) ) {
            writeHeader("Connection", "keep-alive");
        } else {
            m_last = true;
            writeHeader("Connection", "close");
        }
    }

    if ( !m_sentContentLengthHeader && !m_sentTransferEncodingHeader ) {
        if ( m_useChunkedEncoding )
            writeHeader("Transfer-Encoding", "chunked");
        else
            m_last = true;
    }

    // Sun, 06 Nov 1994 08:49:37 GMT - RFC 822. Use QLocale::c() so english is used for month and
    // day.
    if ( !m_sentDate ) {
        QString dateString = QLocale::c().toString(
                                 QDateTime::currentDateTimeUtc(),
                                 "ddd, dd MMM yyyy hh:mm:ss"
                                 ).append(" GMT");
        writeHeader("Date", dateString.toLatin1());
    }
}

///////////////////////////////////////////////////////////////////////////////
