#include "private/qhttpclientrequest_private.hpp"

#include "http-parser/http_parser.h"
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
QHttpRequest::QHttpRequest(QTcpSocket* socket)
    : QHttpAbstractOutput(socket) , d_ptr(new QHttpRequestPrivate(socket, this)) {
    QHTTP_LINE_LOG
}

QHttpRequest::QHttpRequest(QHttpRequestPrivate& dd, QTcpSocket* socket)
    : QHttpAbstractOutput(socket) , d_ptr(&dd) {
    QHTTP_LINE_LOG
}

QHttpRequest::~QHttpRequest() {
    QHTTP_LINE_LOG
}

void
QHttpRequest::setVersion(const QString &versionString) {
    d_func()->iversion  = versionString;
}

void
QHttpRequest::addHeader(const QByteArray &field, const QByteArray &value) {
    d_func()->addHeader(field, value);
}

THeaderHash&
QHttpRequest::headers() {
    return d_func()->iheaders;
}

void
QHttpRequest::write(const QByteArray &data) {
    d_func()->writeData(data);
}

void
QHttpRequest::end(const QByteArray &data) {
    Q_D(QHttpRequest);

    if ( d->endPacket(data) )
        emit done(false);   // should wait for response packet
}

///////////////////////////////////////////////////////////////////////////////
void
QHttpRequestPrivate::ensureWritingHeaders() {
    if ( ifinished    ||    iheaderWritten )
        return;

    writeRaw(QString("%1 %2 HTTP/%3\r\n")
             .arg(http_method_str(static_cast<http_method>(imethod)))
             .arg(iurl.path())
             .arg(iversion)
             .toLatin1()
             );
    writeHeaders();
    writeRaw("\r\n");
    isocket->flush();

    iheaderWritten = true;
}

void
QHttpRequestPrivate::writeHeaders() {
    if ( ifinished    ||    iheaderWritten )
        return;

    if ( iheaders.keyHasValue("connection", "keep-alive") )
        ikeepAlive = true;
    else
        iheaders.insert("connection", "close");

    if ( !iheaders.contains("host") ) {
        iheaders.insert("host",
                        QString("%1:%2").arg(iurl.host()).arg(iurl.port()).toLatin1()
                        );
    }

    for ( THeaderHash::const_iterator cit = iheaders.begin(); cit != iheaders.end(); cit++ ) {
        const QByteArray& field = cit.key();
        const QByteArray& value = cit.value();

        writeHeader(field, value);
    }
}

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
