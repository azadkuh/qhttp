#ifndef QHTTPREQUEST_PRIVATE_HPP
#define QHTTPREQUEST_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttprequest.hpp"
#include "qhttpconnection.hpp"
///////////////////////////////////////////////////////////////////////////////
class QHttpRequest::Private
{
public:
    explicit Private(QHttpConnection* connection) :
        m_connection(connection), m_url("http://localhost/"), m_success(false) {
    }

public:
    QHttpConnection*         m_connection;
    THeaderHash              m_headers;
    QHttpRequest::HttpMethod m_method;
    QUrl                     m_url;
    QString                  m_version;
    QString                  m_remoteAddress;
    quint16                  m_remotePort;
    bool                     m_success;
};

///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPREQUEST_PRIVATE_HPP
