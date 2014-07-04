#ifndef QHTTPREQUEST_PRIVATE_HPP
#define QHTTPREQUEST_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpbase.hpp"
#include "qhttprequest.hpp"
#include "qhttpconnection.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
class QHttpRequest::Private : public HttpRequestBase
{
public:
    explicit Private() : isuccessful(false) {
    }

    virtual ~Private() {
    }

public:
    QString                  iversion;
    QString                  iremoteAddress;
    quint16                  iremotePort;
    bool                     isuccessful;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPREQUEST_PRIVATE_HPP
