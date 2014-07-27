#include "clients.hpp"
#include "client.hpp"
#include <QThread>

using namespace qhttp::client;
///////////////////////////////////////////////////////////////////////////////

class ClientsPrivate
{
    Q_DECLARE_PUBLIC(Clients)
    Clients* const      q_ptr;

    static const size_t KThreadCount = 4;
    QThread             ithreads[KThreadCount];

public:
    explicit    ClientsPrivate(Clients* q) : q_ptr(q) {
    }

    virtual    ~ClientsPrivate() {
        for ( size_t i = 0;    i < KThreadCount;    i++ ) {
            ithreads[i].quit();
        }

        for ( size_t i = 0;    i < KThreadCount;    i++ ) {
            ithreads[i].wait(10000);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////
Clients::Clients(QObject *parent) : QObject(parent), d_ptr(new ClientsPrivate(this)) {
}

Clients::~Clients() {
}

bool
Clients::setup(qhttp::TBackend backend,
               const QString &address, quint16 port,
               quint32 count, quint32 timeOut) {
    if ( timeOut == 0 )
        timeOut = 10;
    else if ( timeOut > 10000 )
        timeOut = 100;

    if ( count == 0 )
        return false;
    else if ( count > 1000 )
        count = 1000;


    for ( size_t i = 0;    i < count;    i++ ) {
        QThread* th = &d_func()->ithreads[i % ClientsPrivate::KThreadCount];
        Client* cli = new Client(i+1, 0);
        cli->setup(th);

        cli->ibackend   = backend;
        cli->iport      = port;
        cli->iaddress   = address;
        cli->itimeOut   = timeOut;

        //cli->start();
    }

    for ( size_t i = 0;    i < ClientsPrivate::KThreadCount;    i++ )
        d_func()->ithreads[i].start();

    return true;
}
