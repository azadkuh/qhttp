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

///////////////////////////////////////////////////////////////////////////////
QHttpRequest::QHttpRequest(QHttpConnection *connection)
    : QObject(connection), pimp(nullptr) {
    pimp    = new Private(connection);
}

QHttpRequest::~QHttpRequest() {
    if ( pimp != nullptr ) {
        delete pimp;
        pimp = nullptr;
    }
}

QString
QHttpRequest::MethodToString(HttpMethod method) {
    int index = staticMetaObject.indexOfEnumerator("HttpMethod");
    return staticMetaObject.enumerator(index).valueToKey(method);
}

QHttpRequest::HttpMethod
QHttpRequest::method() const {
    return pimp->m_method;
}

const QString
QHttpRequest::methodString() const {
    return MethodToString(method());
}

const QUrl&
QHttpRequest::url() const {
    return pimp->m_url;
}

const QString
QHttpRequest::path() const {
    return pimp->m_url.path();
}

const QString&
QHttpRequest::httpVersion() const {
    return pimp->m_version;
}

const THeaderHash&
QHttpRequest::headers() const {
    return pimp->m_headers;
}

QString
QHttpRequest::header(const QByteArray &field) {
    return pimp->m_headers.value(field.toLower(), "");
}

const QString&
QHttpRequest::remoteAddress() const {
    return pimp->m_remoteAddress;
}

quint16
QHttpRequest::remotePort() const {
    return pimp->m_remotePort;
}

bool
QHttpRequest::successful() const {
    return pimp->m_success;
}
