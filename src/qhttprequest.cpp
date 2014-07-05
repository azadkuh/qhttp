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
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
QHttpRequest::QHttpRequest(QTcpSocket *socket) : QHttpAbstractInput(socket) {
    d_ptr.reset(new QHttpRequestPrivate(socket, this));
    QHTTP_LINE_LOG
}

QHttpRequest::QHttpRequest(QHttpRequestPrivate &dd, QTcpSocket *socket)
    : QHttpAbstractInput(socket), d_ptr(&dd) {
    QHTTP_LINE_LOG
}

QHttpRequest::~QHttpRequest() {
    QHTTP_LINE_LOG
}

THttpMethod
QHttpRequest::method() const {
    return d_func()->imethod;
}

const QString
QHttpRequest::methodString() const {
    return http_method_str(static_cast<http_method>(d_func()->imethod));
}

const QUrl&
QHttpRequest::url() const {
    return d_func()->iurl;
}

const QString&
QHttpRequest::httpVersion() const {
    return d_func()->iversion;
}

const THeaderHash&
QHttpRequest::headers() const {
    return d_func()->iheaders;
}

const QString&
QHttpRequest::remoteAddress() const {
    return d_func()->iremoteAddress;
}

quint16
QHttpRequest::remotePort() const {
    return d_func()->iremotePort;
}

bool
QHttpRequest::isSuccessful() const {
    return d_func()->isuccessful;
}

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
