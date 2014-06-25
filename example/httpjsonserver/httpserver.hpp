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
    virtual void incomingRequest(QHttpRequest*, QHttpResponse*);

    uint32_t     icounter;

};

///////////////////////////////////////////////////////////////////////////////

class ClientConnection : public QObject
{
    Q_OBJECT

public:
    explicit    ClientConnection(QHttpRequest*, QHttpResponse*, QObject*);
    virtual    ~ClientConnection();

signals:
    void        requestQuit();

protected:
    void        onComplete();

protected:
    QHttpRequest*   ireq;
    QHttpResponse*  iresp;
    QByteArray      ibody;
};

///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////
#endif // HTTPSERVER_HPP
