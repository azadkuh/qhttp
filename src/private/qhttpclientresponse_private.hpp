/** private imeplementation.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPCLIENT_RESPONSE_PRIVATE_HPP
#define QHTTPCLIENT_RESPONSE_PRIVATE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpbase.hpp"
#include "qhttpclient.hpp"
#include "qhttpclientresponse.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
class QHttpResponsePrivate : public HttpResponseBase
{
public:
    explicit    QHttpResponsePrivate(QHttpClient* cli, QHttpResponse* q)
        : iclient(cli), q_ptr(q) {
        QHTTP_LINE_DEEPLOG
    }

    virtual    ~QHttpResponsePrivate() {
        QHTTP_LINE_DEEPLOG
    }

    void       initialize() {
    }

public:
    bool                    isuccessful = false;
    QString                 icustomeStatusMessage;

protected:
    QHttpClient* const      iclient;
    QHttpResponse* const    q_ptr;
    Q_DECLARE_PUBLIC(QHttpResponse)
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // QHTTPCLIENT_RESPONSE_PRIVATE_HPP
