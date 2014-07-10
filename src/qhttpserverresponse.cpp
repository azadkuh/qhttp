#include "private/qhttpserverresponse_private.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
QHttpResponse::QHttpResponse(QTcpSocket* socket)
    : QHttpAbstractOutput(socket) , d_ptr(new QHttpResponsePrivate(socket, this)) {
    d_ptr->initialize();
    QHTTP_LINE_LOG
}

QHttpResponse::QHttpResponse(QHttpResponsePrivate& dd, QTcpSocket* socket)
    : QHttpAbstractOutput(socket) , d_ptr(&dd) {
    d_ptr->initialize();
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

QHttpConnection*
QHttpResponse::connection() const {
    return static_cast<QHttpConnection*>(d_func()->isocket);
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
