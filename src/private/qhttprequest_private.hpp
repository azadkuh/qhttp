#ifndef QHTTPREQUEST_PRIVATE_HPP
#define QHTTPREQUEST_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpbase.hpp"
#include "qhttprequest.hpp"
#include "qhttpconnection.hpp"
///////////////////////////////////////////////////////////////////////////////
class QHttpRequest::Private : public HttpRequestBase
{
public:
    explicit Private() : isuccessful(false) {
    }

public:
    QString                  iversion;
    QString                  iremoteAddress;
    quint16                  iremotePort;
    bool                     isuccessful;
};

///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPREQUEST_PRIVATE_HPP
