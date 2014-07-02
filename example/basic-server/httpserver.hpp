#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

///////////////////////////////////////////////////////////////////////////////
#include "qhttpserver.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////
/** a simple HTTP server who collects body data and gives response. */
class HttpServer : public QHttpServer
{
    Q_OBJECT

public:
    explicit        HttpServer(QObject *parent);
    virtual        ~HttpServer();

signals:
    /** if a client sends an HTTP header as "command: quit", this server stops and emit this signal. */
    void            closed();

protected:
    virtual void    incomingConnection(QHttpConnection* connection);    ///< overload

    uint32_t        icounter;   ///< a dumb counter for incomming requests.
};
///////////////////////////////////////////////////////////////////////////////
/** connection class for gathering incomming chunks of data. */
class ClientConnection : public QObject
{
    Q_OBJECT

public:
    explicit        ClientConnection(uint32_t id, QHttpConnection*);
    void            processRequest(QHttpRequest*, QHttpResponse*);

signals:
    void            requestQuit();

protected:
    uint32_t        iconnectionId;
    QByteArray      ibody;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////
#endif // HTTPSERVER_HPP
