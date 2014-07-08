#ifndef QHTTP_CLIENT_HPP
#define QHTTP_CLIENT_HPP

#define QHTTPCLIENT_VERSION_MAJOR 1
#define QHTTPCLIENT_VERSION_MINOR 0
#define QHTTPCLIENT_VERSION_PATCH 0

///////////////////////////////////////////////////////////////////////////////
#include "qhttpserverfwd.hpp"

#include <QObject>
#include <QUrl>
///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace client {
///////////////////////////////////////////////////////////////////////////////

/** */
class QHttpClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 timeOut READ timeOut WRITE setTimeOut)

public:
    /** . */
    explicit    QHttpClient(QObject *parent = nullptr);

    virtual    ~QHttpClient();

    void        request(THttpMethod method, QUrl url);

    bool        isOpen() const;

    void        close();



    /** returns time-out value [mSec] for open connections (sockets).
     *  @sa setTimeOut(). */
    quint32     timeOut()const;

    /** set time-out for new open connections in miliseconds [mSec].
     * each connection will be forcefully closed after this timeout.
     *  a zero (0) value disables timer for new connections. */
    void        setTimeOut(quint32);

signals:
    void        connected(QHttpRequest* req);

    void        newResponse(QHttpResponse* res);

    void        disconnected();

protected:
    explicit    QHttpClient(QHttpClientPrivate&, QObject*);

    void        timerEvent(QTimerEvent*);

    Q_DECLARE_PRIVATE(QHttpClient)
    Q_DISABLE_COPY(QHttpClient)
    QScopedPointer<QHttpClientPrivate>  d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace client
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // define QHTTP_CLIENT_HPP
