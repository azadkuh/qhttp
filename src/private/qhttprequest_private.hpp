#ifndef QHTTPREQUEST_PRIVATE_HPP
#define QHTTPREQUEST_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttprequest.hpp"
#include "qhttpconnection.hpp"
///////////////////////////////////////////////////////////////////////////////
class QHttpRequest::Private
{
public:
    explicit Private() : isuccessful(false) {
    }

public:
    THeaderHash              iheaders;
    THttpMethod              imethod;
    QUrl                     iurl;
    QString                  iversion;
    QString                  iremoteAddress;
    quint16                  iremotePort;
    bool                     isuccessful;
};

///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPREQUEST_PRIVATE_HPP
