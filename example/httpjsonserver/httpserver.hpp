#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

///////////////////////////////////////////////////////////////////////////////
#include "qhttpserver.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////

class HttpServer : public QHttpServer
{
    Q_OBJECT

public:
    explicit     HttpServer(QObject *parent);
    virtual     ~HttpServer();

signals:
    void         closed();

protected:
    void         incomingConnection(QHttpConnection *connection);
};

///////////////////////////////////////////////////////////////////////////////

class ClientConnection : public QObject
{
    Q_OBJECT

public:
    explicit        ClientConnection(QHttpConnection*);
    void            processRequest(QHttpRequest*, QHttpResponse*);

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
