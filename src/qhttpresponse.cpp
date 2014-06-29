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
QHttpResponse::QHttpResponse(QTcpSocket* socket)
    : QObject(socket) , pimp(nullptr) {
    pimp    = new Private(this, socket);

#if QHTTPSERVER_MEMORY_LOG > 0
    fprintf(stderr, "    %s:%s(%d): obj = %p\n", __FILE__, __FUNCTION__, __LINE__, this);
#endif
}

QHttpResponse::~QHttpResponse() {
    if ( pimp != nullptr ) {
        // finish if required
        end();

        delete pimp;
        pimp  = nullptr;
    }

#if QHTTPSERVER_MEMORY_LOG > 0
    fprintf(stderr, "    %s:%s(%d): obj = %p\n", __FILE__, __FUNCTION__, __LINE__, this);
#endif
}

void
QHttpResponse::setHeader(const QByteArray &field, const QByteArray &value) {
    if ( !pimp->ifinished )
        pimp->iheaders[field.toLower()] = value.toLower();
    else
        qWarning() << "QHttpResponse::setHeader() Cannot set headers after response has finished.";
}

void
QHttpResponse::writeHead(int status) {
    if ( pimp->ifinished ) {
        qWarning()
            << "QHttpResponse::writeHead() Cannot write headers after response has finished.";
        return;
    }

    if ( pimp->iheaderWritten ) {
        qWarning() << "QHttpResponse::writeHead() Already called once for this response.";
        return;
    }

    pimp->write(QString("HTTP/1.1 %1 %2\r\n")
                .arg(status)
                .arg(QHttpServer::statusCodes()[status]).toLatin1()
                );
    pimp->writeHeaders();
    pimp->write("\r\n");

    pimp->iheaderWritten = true;
}

void
QHttpResponse::writeHead(StatusCode statusCode) {
    writeHead(static_cast<int>(statusCode));
}

void
QHttpResponse::write(const QByteArray &data) {
    if ( pimp->ifinished ) {
        qWarning() << "QHttpResponse::write() Cannot write body after response has finished.";
        return;
    }

    if ( !pimp->iheaderWritten ) {
        qWarning() << "QHttpResponse::write() You must call writeHead() before writing body data.";
        return;
    }

    pimp->write(data);
}

void
QHttpResponse::end(const QByteArray &data) {
    if ( pimp->ifinished )
        return;


    if ( data.size() > 0 )
        write(data);

    pimp->ifinished = true;

    emit done(!pimp->ikeepAlive);
}

///////////////////////////////////////////////////////////////////////////////
void
QHttpResponse::Private::writeHeader(const QByteArray& field, const QByteArray& value) {
    if ( !ifinished ) {
        QByteArray buffer = QByteArray(field)
                            .append(": ")
                            .append(value)
                            .append("\r\n");
        write(buffer);
    } else
        qWarning()
            << "QHttpResponse::writeHeader() Cannot write headers after response has finished.";
}

void
QHttpResponse::Private::writeHeaders() {
    if ( ifinished )
        return;

    bool bsentConnectionHeader       = false;
    bool bsentTransferEncodingHeader = false;
    bool buseChunkedEncoding         = false;
    bool bsentContentLengthHeader    = false;

    for ( THeaderHash::const_iterator cit = iheaders.begin(); cit != iheaders.end(); cit++ ) {
        const QByteArray& field = cit.key();
        const QByteArray& value = cit.value();

        if ( field == "connection" ) {
            bsentConnectionHeader = true;
            ikeepAlive = (value == "keep-alive");

        } else if ( field == "transfer-encoding" ) {
            bsentTransferEncodingHeader = true;
            if ( value == "chunked" )
                buseChunkedEncoding = true;

        } else if ( field == "content-length" ) {
            bsentContentLengthHeader = true;
        }

        writeHeader(field, value);
    }

    if ( !bsentConnectionHeader ) {
        if ( ikeepAlive && ( bsentContentLengthHeader || buseChunkedEncoding ) ) {
            writeHeader("connection", "keep-alive");
        } else {
            writeHeader("connection", "close");
        }
    }

    if ( !bsentContentLengthHeader && !bsentTransferEncodingHeader ) {
        if ( buseChunkedEncoding )
            writeHeader("transfer-encoding", "chunked");
        else
            ikeepAlive = false;
    }

    if ( !iheaders.contains("date") ) {
        // Sun, 06 Nov 1994 08:49:37 GMT - RFC 822. Use QLocale::c() so english is used for month and
        // day.
        QString dateString = QLocale::c().toString(
                                 QDateTime::currentDateTimeUtc(),
                                 "ddd, dd MMM yyyy hh:mm:ss GMT"
                                 );
        writeHeader("date", dateString.toLatin1());
    }
}

///////////////////////////////////////////////////////////////////////////////
