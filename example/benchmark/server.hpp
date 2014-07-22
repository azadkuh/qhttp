#ifndef SERVER_HPP
#define SERVER_HPP

#include "qhttpserver.hpp"

///////////////////////////////////////////////////////////////////////////////
class ServerPrivate;

class Server : public qhttp::server::QHttpServer
{
public:
    explicit    Server(QObject* parent);
    virtual    ~Server();

protected:
    void        incomingConnection(qintptr handle) override;
    void        timerEvent(QTimerEvent *) override;

    Q_DECLARE_PRIVATE(Server)
    Q_DISABLE_COPY(Server)
    QScopedPointer<ServerPrivate>   d_ptr;
};

///////////////////////////////////////////////////////////////////////////////
#endif // SERVER_HPP
