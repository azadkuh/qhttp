#include "private/qhttpclientrequest_private.hpp"
#include "qhttpclient.hpp"
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
QHttpRequest::QHttpRequest(QHttpClient* cli)
    : QHttpAbstractOutput(cli) , d_ptr(new QHttpRequestPrivate(cli, this)) {
    d_ptr->initialize();
    QHTTP_LINE_LOG
}

QHttpRequest::QHttpRequest(QHttpRequestPrivate& dd, QHttpClient* cli)
    : QHttpAbstractOutput(cli) , d_ptr(&dd) {
    d_ptr->initialize();
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
        emit done(!d->ikeepAlive);

    deleteLater();
}

QHttpClient*
QHttpRequest::connection() const {
    return d_func()->iclient;
}

///////////////////////////////////////////////////////////////////////////////
void
QHttpRequestPrivate::ensureWritingHeaders() {
    if ( ifinished    ||    iheaderWritten )
        return;

    QByteArray title;
    title.reserve(512);
    title.append(qhttp::Stringify::toString(imethod))
            .append(" ");

    QByteArray path = iurl.path(QUrl::FullyEncoded).toLatin1();
    if ( path.size() == 0 )
        path = "/";
    title.append(path);

    if ( iurl.hasQuery() )
        title.append("?").append(iurl.query(QUrl::FullyEncoded).toLatin1());


    title.append(" HTTP/")
            .append(iversion.toLatin1())
            .append("\r\n");

    writeRaw(title);
    writeHeaders();

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
        quint16 port = iurl.port();
        if ( port == 0 )
            port = 80;

        iheaders.insert("host",
                        QString("%1:%2").arg(iurl.host()).arg(port).toLatin1()
                        );
    }

    for ( auto cit = iheaders.constBegin(); cit != iheaders.constEnd(); cit++ ) {
        const QByteArray& field = cit.key();
        const QByteArray& value = cit.value();

        writeHeader(field, value);
    }

    writeRaw("\r\n");

    if ( itcpSocket )
        itcpSocket->flush();
    else if ( ilocalSocket )
        ilocalSocket->flush();
}

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
