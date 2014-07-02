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
#include "private/qhttprequest_private.hpp"
#include "http-parser/http_parser.h"

#include <QTcpSocket>
///////////////////////////////////////////////////////////////////////////////
QHttpRequest::QHttpRequest(QTcpSocket *socket)
    : QObject(socket), pimp(nullptr) {
    pimp    = new Private();

    QObject::connect(socket, &QTcpSocket::disconnected, [this](){
        deleteLater();
    });

#if QHTTPSERVER_MEMORY_LOG > 0
    fprintf(stderr, "    %s:%s(%d): obj = %p\n", __FILE__, __FUNCTION__, __LINE__, this);
#endif
}

QHttpRequest::~QHttpRequest() {
    if ( pimp != nullptr ) {
        delete pimp;
        pimp = nullptr;
    }

#if QHTTPSERVER_MEMORY_LOG > 0
    fprintf(stderr, "    %s:%s(%d): obj = %p\n", __FILE__, __FUNCTION__, __LINE__, this);
#endif
}

QString
QHttpRequest::MethodToString(THttpMethod method) {
    return http_method_str(static_cast<http_method>(method));
}

THttpMethod QHttpRequest::method() const {
    return pimp->imethod;
}

const QString
QHttpRequest::methodString() const {
    return MethodToString(method());
}

const QUrl&
QHttpRequest::url() const {
    return pimp->iurl;
}

const QString&
QHttpRequest::httpVersion() const {
    return pimp->iversion;
}

const THeaderHash&
QHttpRequest::headers() const {
    return pimp->iheaders;
}

const QString&
QHttpRequest::remoteAddress() const {
    return pimp->iremoteAddress;
}

quint16
QHttpRequest::remotePort() const {
    return pimp->iremotePort;
}

bool
QHttpRequest::isSuccessful() const {
    return pimp->isuccessful;
}

void
QHttpRequest::setHeader(const QByteArray &field, const QByteArray &value) {
    pimp->iheaders.insert(field.toLower(), value.toLower());
}
