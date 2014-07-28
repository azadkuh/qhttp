#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include <QObject>
#include "qhttpfwd.hpp"

///////////////////////////////////////////////////////////////////////////////

class ClientsPrivate;

class Clients : public QObject
{
public:
    explicit    Clients(size_t threads, QObject *parent);
    virtual    ~Clients();

    bool        setup(qhttp::TBackend backend,
                      const QString& address, quint16 port,
                      quint32 count, quint32 timeOut);

protected:
    Q_DECLARE_PRIVATE(Clients)
    Q_DISABLE_COPY(Clients)
    QScopedPointer<ClientsPrivate> d_ptr;
};

///////////////////////////////////////////////////////////////////////////////

#endif // CLIENTS_HPP
