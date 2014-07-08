#ifndef QHTTP_CLIENT_REQUEST_HPP
#define QHTTP_CLIENT_REQUEST_HPP

///////////////////////////////////////////////////////////////////////////////
#include "qhttpabstracts.hpp"
#include <QUrl>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////
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
#endif // define QHTTP_CLIENT_REQUEST_HPP
