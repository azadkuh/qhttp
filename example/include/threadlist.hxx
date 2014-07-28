#ifndef THREADLIST_HXX
#define THREADLIST_HXX

#include <QThread>
#include <QVector>
///////////////////////////////////////////////////////////////////////////////
class ThreadList
{
    QVector<QThread*>   ithreads;

public:
    ~ThreadList() {
        deleteAll();
    }

    void       create(size_t count) {
        deleteAll();
        for ( size_t i = 0;    i < count;    i++ ) {
            ithreads.append( new QThread() );
        }
    }

    size_t      size()const {
        return ithreads.size();
    }

    QThread*    at(size_t index) {
        int size = ithreads.size();
        if ( (int)index >= size )
            index = index % size;

        return ithreads.at(index);
    }

    void        startAll() {
        foreach (QThread* th, ithreads) {
            th->start();
        }
    }

    void        quitAll() {
        foreach (QThread* th, ithreads) {
            th->quit();
        }
    }

    void        joinAll(unsigned long timeout = ULONG_MAX) {
        foreach (QThread* th, ithreads) {
            th->wait(timeout);
        }
    }

    void        deleteAll() {
        quitAll();
        joinAll(5000);

        foreach (QThread* th, ithreads) {
            th->deleteLater();
        }

        ithreads.clear();
    }
};


///////////////////////////////////////////////////////////////////////////////
#endif // THREADLIST_HXX
