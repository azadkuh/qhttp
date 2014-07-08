#ifndef QHTTPCLIENT_REQUEST_HPP
#define QHTTPCLIENT_REQUEST_HPP

///////////////////////////////////////////////////////////////////////////////
#include "qhttpabstracts.hpp"
#include <QUrl>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
/** a class for building a new HTTP request.
 * the life cycle of this class and the memory management is handled by QHttpClient.
 * @sa QHttpClient
 */
class QHttpRequest : public QHttpAbstractOutput
{
    Q_OBJECT

public:
    virtual        ~QHttpRequest();

public: // QHttpAbstractOutput methods:
    /** @see QHttpAbstractOutput::setVersion(). */
    void            setVersion(const QString& versionString);

    /** @see QHttpAbstractOutput::addHeader(). */
    void            addHeader(const QByteArray& field, const QByteArray& value);

    /** @see QHttpAbstractOutput::headers(). */
    THeaderHash&    headers();

    /** @see QHttpAbstractOutput::write(). */
    void            write(const QByteArray &data);

    /** @see QHttpAbstractOutput::end(). */
    void            end(const QByteArray &data = QByteArray());


protected:
    explicit        QHttpRequest(QTcpSocket*);
    explicit        QHttpRequest(QHttpRequestPrivate&, QTcpSocket*);
    friend class    QHttpClientPrivate;

    Q_DECLARE_PRIVATE(QHttpRequest)
    QScopedPointer<QHttpRequestPrivate> d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // define QHTTPCLIENT_REQUEST_HPP
