#include "private/qhttpclientrequest_private.hpp"
#include "qhttpclient.hpp"
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
QHttpRequest::QHttpRequest(QHttpClient* cli)
    : QHttpAbstractOutput(cli) , pPrivate(new QHttpRequestPrivate(cli, this)) {
    this->pPrivate->initialize();
    QHTTP_LINE_LOG
}

QHttpRequest::QHttpRequest(QHttpRequestPrivate& dd, QHttpClient* cli)
    : QHttpAbstractOutput(cli) , pPrivate(&dd) {
    this->pPrivate->initialize();
    QHTTP_LINE_LOG
}

QHttpRequest::~QHttpRequest() {
    QHTTP_LINE_LOG
}

void
QHttpRequest::setVersion(const QString &versionString) {
   this->pPrivate->iversion  = versionString;
}

void
QHttpRequest::addHeader(const QByteArray &field, const QByteArray &value) {
    this->pPrivate->addHeader(field, value);
}

THeaderHash&
QHttpRequest::headers() {
    return this->pPrivate->iheaders;
}

void
QHttpRequest::write(const QByteArray &data) {
    this->pPrivate->writeData(data);
}

void
QHttpRequest::end(const QByteArray &data) {
    if ( this->pPrivate->endPacket(data) )
        emit done(!this->pPrivate->ikeepAlive);
}

QHttpClient*
QHttpRequest::connection() const {
    return this->pPrivate->iclient;
}

///////////////////////////////////////////////////////////////////////////////
QByteArray
QHttpRequestPrivate::makeTitle() {

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

    return title;
}

void
QHttpRequestPrivate::prepareHeadersToWrite() {

    if ( !iheaders.contains("host") ) {
        int port = iurl.port();
        if ( port == 0 )
            port = 80;

        iheaders.insert("host",
                        QString("%1:%2").arg(iurl.host()).arg(port).toLatin1()
                        );
    }
}

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
