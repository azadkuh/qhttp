/** private imeplementation.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPSERVER_PRIVATE_HPP
#define QHTTPSERVER_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////

class QHttpServerPrivate
{
public:
    quint32         itimeOut = 0;
    TServerHandler  ihandler = nullptr;

public:
    explicit    QHttpServerPrivate() {
        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpServerPrivate() {
        QHTTP_LINE_DEEPLOG
    }
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////

#endif // QHTTPSERVER_PRIVATE_HPP
