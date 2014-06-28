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

#include "private/qhttpconnection_private.hpp"
#include <QFile>
///////////////////////////////////////////////////////////////////////////////

QHttpConnection::QHttpConnection(qintptr handle, QObject *parent, quint32 timeOut)
    : QObject(parent), pimp(nullptr) {
    pimp    = new Private(handle, this, timeOut);

#if QHTTPSERVER_MEMORY_LOG > 0
    fprintf(stderr, "%s:%s(%d): obj = %p\n", __FILE__, __FUNCTION__, __LINE__, this);
#endif
}

QHttpConnection::~QHttpConnection() {
    if ( pimp != nullptr ) {
        delete pimp;
        pimp = nullptr;
    }

#if QHTTPSERVER_MEMORY_LOG > 0
    fprintf(stderr, "%s:%s(%d): obj = %p\n", __FILE__, __FUNCTION__, __LINE__, this);
#endif
}

QHttpRequest*
QHttpConnection::latestRequest() const {
    return pimp->m_request;
}

QHttpResponse*
QHttpConnection::latestResponse() const {
    return pimp->m_response;
}

void
QHttpConnection::timerEvent(QTimerEvent *) {
    pimp->m_socket->disconnectFromHost();
}

///////////////////////////////////////////////////////////////////////////////

void
QHttpConnection::Private::parseRequest() {
    Q_ASSERT(m_parser);

    while (m_socket->bytesAvailable()) {
        char buffer[4096] = {0};
        size_t readLength = m_socket->read(buffer, 4095);

#       if QHTTPSERVER_MESSAGES_LOG > 0
        m_inputBuffer.append(buffer);
#       endif

        http_parser_execute(m_parser, m_parserSettings,
                            buffer, readLength);
    }
}

///////////////////////////////////////////////////////////////////////////////

int
QHttpConnection::Private::messageBegin(http_parser*) {
    m_currentHeaders.clear();
    m_currentUrl.clear();
    m_currentUrl.reserve(128);

    m_request = new QHttpRequest(iparent);
    return 0;
}

int
QHttpConnection::Private::url(http_parser*, const char* at, size_t length) {
    Q_ASSERT(m_request);

    m_currentUrl.append(at, length);
    return 0;
}

int
QHttpConnection::Private::headerField(http_parser*, const char* at, size_t length) {
    Q_ASSERT(m_request);

    // insert the header we parsed previously
    // into the header map
    if ( !m_currentHeaderField.isEmpty() && !m_currentHeaderValue.isEmpty() ) {
        // header names are always lower-cased
        m_currentHeaders.insert(
                    m_currentHeaderField.toLower(),
                    m_currentHeaderValue.toLower()
                    );
        // clear header value. this sets up a nice
        // feedback loop where the next time
        // HeaderValue is called, it can simply append
        m_currentHeaderField.clear();
        m_currentHeaderValue.clear();
    }

    m_currentHeaderField.append(at, length);
    return 0;
}

int
QHttpConnection::Private::headerValue(http_parser*, const char* at, size_t length) {
    Q_ASSERT(m_request);

    m_currentHeaderValue.append(at, length);
    return 0;
}

int
QHttpConnection::Private::headersComplete(http_parser* parser) {
    Q_ASSERT(m_request);

    // get parsed url
    struct http_parser_url urlInfo;
    int r = http_parser_parse_url(m_currentUrl.constData(),
                                  m_currentUrl.size(),
                                  parser->method == HTTP_CONNECT,
                                  &urlInfo);
    Q_ASSERT(r == 0);
    Q_UNUSED(r);

    m_request->pimp->m_url = createUrl(
                                 m_currentUrl.constData(),
                                 urlInfo
                                 );

    // set method
    m_request->pimp->m_method =
            static_cast<QHttpRequest::HttpMethod>(parser->method);

    // set version
    m_request->pimp->m_version = QString("%1.%2")
                                 .arg(parser->http_major)
                                 .arg(parser->http_minor);

    // Insert last remaining header
    m_currentHeaders.insert(
                m_currentHeaderField.toLower(),
                m_currentHeaderValue.toLower()
                );
    m_request->pimp->m_headers       = m_currentHeaders;

    // set client information
    m_request->pimp->m_remoteAddress = m_socket->peerAddress().toString();
    m_request->pimp->m_remotePort    = m_socket->peerPort();



    m_response = new QHttpResponse(m_socket);

    if ( parser->http_major < 1 || parser->http_minor < 1 ||
          m_currentHeaders.value("connection", "") == "close" ) {

        m_response->pimp->m_keepAlive = false;
        m_response->pimp->m_last      = true;
    }

    // we are good to go!
    emit iparent->newRequest(m_request, m_response);
    return 0;
}

int
QHttpConnection::Private::body(http_parser*, const char* at, size_t length) {
    Q_ASSERT(m_request);

    emit m_request->data(QByteArray(at, length));
    return 0;
}

int
QHttpConnection::Private::messageComplete(http_parser*) {
    Q_ASSERT(m_request);

#   if QHTTPSERVER_MESSAGES_LOG > 0
    QFile f("/tmp/incomingMessages.log");
    if ( f.open(QIODevice::Append | QIODevice::WriteOnly) ) {
        f.write(m_inputBuffer);
        f.write("\n---------------------\n");
        f.flush();
    }
#   endif

    m_request->pimp->m_success = true;
    emit m_request->end();
    return 0;
}


/* URL Utilities */
#define HAS_URL_FIELD(info, field) (info.field_set &(1 << (field)))

#define GET_FIELD(data, info, field)                                                               \
    QString::fromLatin1(data + info.field_data[field].off, info.field_data[field].len)

#define CHECK_AND_GET_FIELD(data, info, field)                                                     \
    (HAS_URL_FIELD(info, field) ? GET_FIELD(data, info, field) : QString())

QUrl
QHttpConnection::Private::createUrl(const char *urlData, const http_parser_url &urlInfo) {
    QUrl url;
    url.setScheme(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_SCHEMA));
    url.setHost(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_HOST));
    // Port is dealt with separately since it is available as an integer.
    url.setPath(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_PATH));
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    url.setQuery(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_QUERY));
#else
    if (HAS_URL_FIELD(urlInfo, UF_QUERY)) {
        url.setEncodedQuery(QByteArray(urlData + urlInfo.field_data[UF_QUERY].off,
                                       urlInfo.field_data[UF_QUERY].len));
    }
#endif
    url.setFragment(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_FRAGMENT));
    url.setUserInfo(CHECK_AND_GET_FIELD(urlData, urlInfo, UF_USERINFO));

    if (HAS_URL_FIELD(urlInfo, UF_PORT))
        url.setPort(urlInfo.port);

    return url;
}

#undef CHECK_AND_SET_FIELD
#undef GET_FIELD
#undef HAS_URL_FIELD

///////////////////////////////////////////////////////////////////////////////

