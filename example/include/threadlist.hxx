#ifndef THREADLIST_HXX
#define THREADLIST_HXX

#include <QThread>
///////////////////////////////////////////////////////////////////////////////
template<size_t KCount>
class ThreadList
{
    QThread         ithreads[KCount];

public:
    ~ThreadList() {
        quitAll();
        joinAll(5000);
    }

    QThread*    at(size_t index) {
        if ( index >= KCount )
            index = index % KCount;

        return &ithreads[index];
    }

    void        startAll() {
        for ( size_t i = 0;    i < KCount;    i++ ) {
            ithreads[i].start();
        }
    }

    void        quitAll() {
        for ( size_t i = 0;    i < KCount;    i++ ) {
            ithreads[i].quit();
        }
    }

    void        joinAll(unsigned long timeout = ULONG_MAX) {
        for ( size_t i = 0;    i < KCount;    i++ ) {
            ithreads[i].wait(timeout);
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
#endif // THREADLIST_HXX
