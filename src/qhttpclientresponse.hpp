/** HTTP response received by client.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPCLIENT_RESPONSE_HPP
#define QHTTPCLIENT_RESPONSE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpabstracts.hpp"

#include <QUrl>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
/** a class for reading incoming HTTP response from a server.
 * the life cycle of this class and the memory management is handled by QHttpClient.
 * @sa QHttpClient
 */
class QHttpResponse : public QHttpAbstractInput
{
    Q_OBJECT

public:
    virtual            ~QHttpResponse();

public: // QHttpAbstractInput methods:
    /** @see QHttpAbstractInput::headers(). */
    const THeaderHash&  headers() const;

    /** @see QHttpAbstractInput::httpVersion(). */
    const QString&      httpVersion() const;

    /** @see QHttpAbstractInput::isSuccessful(). */
    bool                isSuccessful() const;


public:
    /** The status code of this response. */
    TStatusCode         status() const ;

    /** The server status message as string.
     *  may be slightly different than: @code qhttp::Stringify::toString(status()); @endcode
     *  depending on implementation of HTTP server. */
    const QString&      statusString() const;

    /** returns parent QHttpClient object. */
    QHttpClient*        connection() const;

protected:
    explicit            QHttpResponse(QTcpSocket*);
    explicit            QHttpResponse(QHttpResponsePrivate&, QTcpSocket*);
    friend class        QHttpClientPrivate;

    Q_DECLARE_PRIVATE(QHttpResponse)
    QScopedPointer<QHttpResponsePrivate> d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // define QHTTPCLIENT_RESPONSE_HPP
