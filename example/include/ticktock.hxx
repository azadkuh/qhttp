#ifndef TICKTOCK_HXX
#define TICKTOCK_HXX

#if defined(_WINDOWS)
#   include <windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
#   include <mach/mach_time.h>
#elif defined(__GNUC__) || defined(__linux__)
#   include <time.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace am {
///////////////////////////////////////////////////////////////////////////////
#if     defined(__APPLE__) && defined(__MACH__)
class TickTock
{
    uint64_t            itick;
public:
    explicit TickTock() : itick(0) {
    }

    explicit TickTock(bool init) {
        if ( init )
            tick();
    }

    void tick() {
        itick = mach_absolute_time();
    }

    unsigned long tock() const {
        uint64_t el = mach_absolute_time() - itick;
        mach_timebase_info_data_t   timebase;
        mach_timebase_info(&timebase);

        double del = (double)el * (double)timebase.numer /
                     (double)timebase.denom /
                     1.0e6;
        return (unsigned long)del;
    }

    static unsigned long now() {
        return mach_absolute_time();
    }

};

#elif   defined(__GNUC__)
class TickTock
{
    timespec            itick;
    clockid_t           iclock;
public:
    explicit TickTock() {
        iclock = CLOCK_MONOTONIC_RAW;
    }

    explicit TickTock(bool init, clockid_t clk = CLOCK_MONOTONIC) {
        iclock = clk;
        if ( init )
            tick();
    }

    void tick() {
        clock_gettime(iclock, &itick);
    }

    unsigned long tock() const {
        timespec tp;
        clock_gettime(iclock, &tp);
        return ( tp.tv_sec - itick.tv_sec ) * 1000u    +
                ( tp.tv_nsec - itick.tv_nsec ) / 1000000u;
    }

    static unsigned long now(clockid_t clk = CLOCK_MONOTONIC) {
        timespec tp;
        clock_gettime(clk, &tp);
        return (tp.tv_sec) * 1000u + (tp.tv_nsec) / 1000000u;
    }

    /** micro second version of tock. */
    unsigned long utock() const {
        timespec tp;
        clock_gettime(iclock, &tp);
        return ( tp.tv_sec - itick.tv_sec ) * 1000000u  +
                ( tp.tv_nsec - itick.tv_nsec) / 1000u;
    }

    /** micro second version of now */
    static unsigned long unow(clockid_t clk = CLOCK_MONOTONIC) {
        timespec tp;
        clock_gettime(clk, &tp);
        return (tp.tv_sec) * 1000000u + (tp.tv_nsec) / 1000u;
    }
};

#elif   defined(_WINDOWS) || defined(WINCE)
class TickTock
{
    unsigned long       itick;
public:
    explicit TickTock() : itick(0) {
    }

    explicit TickTock(bool init) {
        if ( init )
            tick();
    }

    void tick() {
        itick = ::GetTickCount();
    }

    unsigned long tock() const {
        DWORD el = ::GetTickCount() - itick;
        return el;
    }

    static unsigned long now() {
        return ::GetTickCount();
    }
};


#endif
///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////
#endif
