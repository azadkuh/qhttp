#ifndef QHTTPSERVER_RESPONSE_HPP
#define QHTTPSERVER_RESPONSE_HPP

///////////////////////////////////////////////////////////////////////////////

#include "qhttpabstracts.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
/** The QHttpResponse class handles sending data back to the client as a response to a request.
 * @sa QHttpConnection
 */
class QHttpResponse : public QHttpAbstractOutput
{
    Q_OBJECT

public:
    virtual        ~QHttpResponse();

public:
    /** set the response HTTP status code. @sa TStatusCode.
     * default value is ESTATUS_BAD_REQUEST.
     * @sa write()
     */
    void            setStatusCode(TStatusCode code);

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
    explicit        QHttpResponse(QTcpSocket*);
    explicit        QHttpResponse(QHttpResponsePrivate&, QTcpSocket*);
    friend class    QHttpConnectionPrivate;

    Q_DECLARE_PRIVATE(QHttpResponse)
    QScopedPointer<QHttpResponsePrivate> d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // define QHTTPSERVER_RESPONSE_HPP
