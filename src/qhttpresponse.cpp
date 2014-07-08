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
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
QHttpResponse::QHttpResponse(QTcpSocket* socket)
    : QHttpAbstractOutput(socket) , d_ptr(new QHttpResponsePrivate(socket, this)) {
    QHTTP_LINE_LOG
}

QHttpResponse::QHttpResponse(QHttpResponsePrivate& dd, QTcpSocket* socket)
    : QHttpAbstractOutput(socket) , d_ptr(&dd) {
    QHTTP_LINE_LOG
}

QHttpResponse::~QHttpResponse() {
    QHTTP_LINE_LOG
}

void
QHttpResponse::setStatusCode(TStatusCode code) {
    d_func()->istatus   = code;
}

void
QHttpResponse::setVersion(const QString &versionString) {
    d_func()->iversion  = versionString;
}

void
QHttpResponse::addHeader(const QByteArray &field, const QByteArray &value) {
    d_func()->addHeader(field, value);
}

THeaderHash&
QHttpResponse::headers() {
    return d_func()->iheaders;
}

void
QHttpResponse::write(const QByteArray &data) {
    d_func()->writeData(data);
}

void
QHttpResponse::end(const QByteArray &data) {
    Q_D(QHttpResponse);

    if ( d->endPacket(data) )
        emit done(!d->ikeepAlive);
}

///////////////////////////////////////////////////////////////////////////////
void
QHttpResponsePrivate::ensureWritingHeaders() {
    if ( ifinished    ||    iheaderWritten )
        return;

    writeRaw(QString("HTTP/%1 %2 %3\r\n")
             .arg(iversion)
             .arg(istatus)
             .arg(Stringify::toString(istatus))
             .toLatin1()
             );
    writeHeaders();
    writeRaw("\r\n");
    isocket->flush();

    iheaderWritten = true;
}

void
QHttpResponsePrivate::writeHeaders() {
    if ( ifinished    ||    iheaderWritten )
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
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
