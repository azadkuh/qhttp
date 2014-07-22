/** HTTP connection class.
 * https://github.com/azadkuh/qhttp
 *
 * @author amir zamani
 * @version 2.0.0
 * @date 2014-07-11
  */

#ifndef QHTTPSERVER_CONNECTION_HPP
#define QHTTPSERVER_CONNECTION_HPP
///////////////////////////////////////////////////////////////////////////////
#include "qhttpfwd.hpp"

#include <QTcpSocket>

///////////////////////////////////////////////////////////////////////////////
namespace qhttp {
namespace server {
///////////////////////////////////////////////////////////////////////////////
/** a HTTP connection in the server.
 * this class controls the HTTP connetion and handles life cycle and the memory management
 *  of QHttpRequest and QHttpResponse instances autoamtically.
 */
class QHttpConnection : public QTcpSocket
{
    Q_OBJECT

public:
    virtual        ~QHttpConnection();

    /** set an optional timer event to close the connection. */
    void            setTimeOut(quint32 miliSeconds);

    /** forcefully kills (closes) a connection. */
    void            killConnection();

    /** optionally set a handler for connection class.
     * @note if you set this handler, the newRequest() signal won't be emitted.
     */
    void            onHandler(const TServerHandler& handler);

signals:
    /** emitted when a pair of HTTP request and response are ready to interact.
     * @param req incoming request by the client.
     * @param res outgoing response to the client.
     */
    void            newRequest(QHttpRequest* req, QHttpResponse* res);

protected:
    explicit        QHttpConnection(QObject *parent);
    explicit        QHttpConnection(QHttpConnectionPrivate&, QObject *);
    void            timerEvent(QTimerEvent*) override;

    Q_DISABLE_COPY(QHttpConnection)
    Q_DECLARE_PRIVATE(QHttpConnection)
    QScopedPointer<QHttpConnectionPrivate>    d_ptr;

    friend class    QHttpServer;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace server
} // namespace qhttp
///////////////////////////////////////////////////////////////////////////////
#endif // #define QHTTPSERVER_CONNECTION_HPP
