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
#include "private/qhttpserver_private.hpp"
#include "qhttpconnection.hpp"
#include "qhttprequest.hpp"
#include "qhttpresponse.hpp"

#include <QTcpSocket>
#include <QVariant>
#include <QDebug>

#include "http-parser/http_parser.h"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
#define HTTP_STATUS_MAP(XX)                    \
    XX(100, "Continue")                        \
    XX(101, "Switching Protocols")             \
    /* RFC 2518) obsoleted by RFC 4918 */      \
    XX(102, "Processing")                      \
    XX(200, "OK")                              \
    XX(201, "Created")                         \
    XX(202, "Accepted")                        \
    XX(203, "Non-Authoritative Information")   \
    XX(204, "No Content")                      \
    XX(205, "Reset Content")                   \
    XX(206, "Partial Content")                 \
    /* RFC 4918 */                             \
    XX(207, "Multi-Status")                    \
    XX(300, "Multiple Choices")                \
    XX(301, "Moved Permanently")               \
    XX(302, "Moved Temporarily")               \
    XX(303, "See Other")                       \
    XX(304, "Not Modified")                    \
    XX(305, "Use Proxy")                       \
    XX(307, "Temporary Redirect")              \
    XX(400, "Bad Request")                     \
    XX(401, "Unauthorized")                    \
    XX(402, "Payment Required")                \
    XX(403, "Forbidden")                       \
    XX(404, "Not Found")                       \
    XX(405, "Method Not Allowed")              \
    XX(406, "Not Acceptable")                  \
    XX(407, "Proxy Authentication Required")   \
    XX(408, "Request Time-out")                \
    XX(409, "Conflict")                        \
    XX(410, "Gone")                            \
    XX(411, "Length Required")                 \
    XX(412, "Precondition Failed")             \
    XX(413, "Request Entity Too Large")        \
    XX(414, "Request-URI Too Large")           \
    XX(415, "Unsupported Media Type")          \
    XX(416, "Requested Range Not Satisfiable") \
    XX(417, "Expectation Failed")              \
    /* RFC 2324 */                             \
    XX(418, "I\"m a teapot")                   \
    /* RFC 4918 */                             \
    XX(422, "Unprocessable Entity")            \
    /* RFC 4918 */                             \
    XX(423, "Locked")                          \
    /* RFC 4918 */                             \
    XX(424, "Failed Dependency")               \
    /* RFC 4918 */                             \
    XX(425, "Unordered Collection")            \
    /* RFC 2817 */                             \
    XX(426, "Upgrade Required")                \
    XX(500, "Internal Server Error")           \
    XX(501, "Not Implemented")                 \
    XX(502, "Bad Gateway")                     \
    XX(503, "Service Unavailable")             \
    XX(504, "Gateway Time-out")                \
    XX(505, "HTTP Version not supported")      \
    /* RFC 2295 */                             \
    XX(506, "Variant Also Negotiates")         \
    /* RFC 4918 */                             \
    XX(507, "Insufficient Storage")            \
    XX(509, "Bandwidth Limit Exceeded")        \
    /* RFC 2774 */                             \
    XX(510, "Not Extended")

#define PATCH_STATUS_CODES(n,s) {n, s},
static struct {
    int         code;
    const char* message;
} g_status_codes[] {
    HTTP_STATUS_MAP(PATCH_STATUS_CODES)
};
#undef PATCH_STATUS_CODES

///////////////////////////////////////////////////////////////////////////////

QHttpServer::QHttpServer(QObject *parent)
    : QTcpServer(parent), d_ptr(new QHttpServerPrivate) {
}

QHttpServer::QHttpServer(QHttpServerPrivate &dd, QObject *parent)
    : QTcpServer(parent), d_ptr(&dd) {
}

QHttpServer::~QHttpServer() {
}

bool
QHttpServer::listen(const QHostAddress& address, quint16 port) {
    return QTcpServer::listen(address, port);
}

const char*
QHttpServer::statusCodeMessage(TStatusCode code) {
    size_t count = sizeof(g_status_codes) / sizeof(g_status_codes[0]);
    for ( size_t i = 0;    i < count;    i++ ) {
        if ( g_status_codes[i].code == code )
            return g_status_codes[i].message;
    }

    return nullptr;
}

const char*
QHttpServer::methodString(THttpMethod method) {
    return http_method_str(static_cast<http_method>(method));
}

quint32
QHttpServer::timeOut() const {
    return d_func()->itimeOut;
}

void
QHttpServer::setTimeOut(quint32 newValue) {
    d_func()->itimeOut = newValue;
}

void
QHttpServer::incomingConnection(qintptr handle) {
    incomingConnection(new QHttpConnection(handle,
                                           this,
                                           d_func()->itimeOut)
                       );
}

void
QHttpServer::incomingConnection(QHttpConnection *connection) {
    QObject::connect(connection,  &QHttpConnection::newRequest,
                     this,        &QHttpServer::newRequest);
}

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
