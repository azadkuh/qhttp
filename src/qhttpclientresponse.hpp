#ifndef QHTTP_CLIENT_RESPONSE_HPP
#define QHTTP_CLIENT_RESPONSE_HPP
///////////////////////////////////////////////////////////////////////////////

#include "qhttpabstracts.hpp"

#include <QUrl>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
class  QHttpResponse : public QHttpAbstractInput
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
    /** The status of the response. */
    TStatusCode         status() const ;

    /** The server status message.
     *  may be slightly different than: @code qhttp::Stringify::toString(status()); @endcode
     *  bases on server status implementation. */
    const QString       statusString() const;




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
#endif // define QHTTP_CLIENT_RESPONSE_HPP
