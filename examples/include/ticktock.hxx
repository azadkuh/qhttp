#ifndef TICKTOCK_HXX
#define TICKTOCK_HXX

#include <chrono>
///////////////////////////////////////////////////////////////////////////////
namespace am {
class TickTock {
public:
    using time_point   = std::chrono::high_resolution_clock::time_point;
    using milliseconds = std::chrono::milliseconds;
    using microseconds = std::chrono::microseconds;

public:
    explicit TickTock(bool initialize) {
        if (initialize)
            tick();
    }

    TickTock()=default;
    ~TickTock()=default;

    static auto now() {
        return std::chrono::high_resolution_clock::now();
    }

    void tick() {
        itick = now();
    }

    template<typename T = milliseconds>
    auto tock() -> typename T::rep {
        return std::chrono::duration_cast<T>(now() - itick).count();
    }

    template<typename T = microseconds>
    auto    tockf() -> double {
        return static_cast<double>(
                    std::chrono::duration_cast<T>(now() - itick).count()
                    );
    }

protected:
    time_point      itick;
};
///////////////////////////////////////////////////////////////////////////////
} // namespace am
///////////////////////////////////////////////////////////////////////////////
#endif
