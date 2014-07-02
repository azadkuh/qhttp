#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

///////////////////////////////////////////////////////////////////////////////
#include "qhttpserver.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////

class HttpServer : public qhttp::server::QHttpServer
{
    Q_OBJECT

public:
    explicit     HttpServer(QObject *parent);
    virtual     ~HttpServer();

signals:
    void         closed();

protected:
    void         incomingConnection(qhttp::server::QHttpConnection *connection);
};

///////////////////////////////////////////////////////////////////////////////

class ClientConnection : public QObject
{
    Q_OBJECT

public:
    explicit        ClientConnection(qhttp::server::QHttpConnection*);
    void            processRequest(qhttp::server::QHttpRequest*, qhttp::server::QHttpResponse*);

signals:
    void            requestQuit();

protected:
    void            onComplete();

protected:
    QByteArray      ibody;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////
#endif // HTTPSERVER_HPP
