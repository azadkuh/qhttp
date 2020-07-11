#include "private/qhttpserverresponse_private.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
QHttpResponse::QHttpResponse(QHttpConnection* conn)
    : QHttpAbstractOutput(conn) , pPrivate(new QHttpResponsePrivate(conn, this)) {
    this->pPrivate->initialize();
    QHTTP_LINE_LOG
}

QHttpResponse::QHttpResponse(QHttpResponsePrivate& dd, QHttpConnection* conn)
    : QHttpAbstractOutput(conn) , pPrivate(&dd) {
    this->pPrivate->initialize();
    QHTTP_LINE_LOG
}

QHttpResponse::~QHttpResponse() {
    QHTTP_LINE_LOG
}

void
QHttpResponse::setStatusCode(TStatusCode code) {
    this->pPrivate->istatus   = code;
}

void
QHttpResponse::setVersion(const QString &versionString) {
    this->pPrivate->iversion  = versionString;
}

void
QHttpResponse::addHeader(const QByteArray &field, const QByteArray &value) {
    this->pPrivate->addHeader(field, value);
}

THeaderHash&
QHttpResponse::headers() {
    return this->pPrivate->iheaders;
}

void
QHttpResponse::write(const QByteArray &data) {
    this->pPrivate->writeData(data);
}

void
QHttpResponse::end(const QByteArray &data) {
    if ( this->pPrivate->endPacket(data) )
        emit done(!this->pPrivate->ikeepAlive);
}

QHttpConnection*
QHttpResponse::connection() const {
    return this->pPrivate->iconnection;
}

///////////////////////////////////////////////////////////////////////////////
QByteArray
QHttpResponsePrivate::makeTitle() {

    QString title = QString("HTTP/%1 %2 %3\r\n")
                    .arg(iversion)
                    .arg(istatus)
                    .arg(Stringify::toString(istatus));

    return title.toLatin1();
}

void
QHttpResponsePrivate::prepareHeadersToWrite() {

    if ( !iheaders.contains("date") ) {
        // Sun, 06 Nov 1994 08:49:37 GMT - RFC 822. Use QLocale::c() so english is used for month and
        // day.
        QString dateString = QLocale::c().
            toString(QDateTime::currentDateTimeUtc(),
                    "ddd, dd MMM yyyy hh:mm:ss")
            .append(" GMT");
        addHeader("date", dateString.toLatin1());
    }
}

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
