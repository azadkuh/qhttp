#ifndef CLIENTS_HPP
#define CLIENTS_HPP

#include <QObject>

///////////////////////////////////////////////////////////////////////////////

class ClientsPrivate;

class Clients : public QObject
{
public:
    explicit    Clients(QObject *parent);
    virtual    ~Clients();

    bool        setup(const QString& address, quint16 port, quint32 count, quint32 timeOut);

protected:
    Q_DECLARE_PRIVATE(Clients)
    Q_DISABLE_COPY(Clients)
    QScopedPointer<ClientsPrivate> d_ptr;
};

///////////////////////////////////////////////////////////////////////////////

#endif // CLIENTS_HPP
