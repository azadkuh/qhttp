#ifndef UNIX_CATCHER_HPP
#define UNIX_CATCHER_HPP

#include <QCoreApplication>

#if defined(Q_OS_UNIX)
#include <signal.h>
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////

inline static void
catchUnixSignals(std::initializer_list<int> quitSignals) {
    auto handler = [](int s) {
        qDebug("\nquit the application (user request signal = %d).\n", s);
        QCoreApplication::flush();
        QCoreApplication::quit();
    };

    struct sigaction sa;
    sa.sa_handler = handler;
    sa.sa_flags   = SA_RESTART;
    sigfillset(&sa.sa_mask);

    for (auto sig : quitSignals)
        sigaction(sig, &sa, nullptr);
}

inline void
catchDefaultOsSignals() {
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP, SIGABRT});
}

///////////////////////////////////////////////////////////////////////////////
#elif defined(Q_OS_WIN32)
#include <ShlObj.h>
///////////////////////////////////////////////////////////////////////////////

inline void
catchWin32Signals() {
    auto handler = [](DWORD sig) -> BOOL {
        bool shouldCleanup =
            (sig == CTRL_C_EVENT) || (sig == CTRL_BREAK_EVENT) ||
            (sig == CTRL_CLOSE_EVENT) || (sig == CTRL_SHUTDOWN_EVENT);

        if (shouldCleanup) {
            fprintf(
                stderr,
                "\nquit the application (user/system request signal = %ld).\n",
                sig);
            fflush(stderr);
            fflush(stdout);
            QCoreApplication::quit();
            return TRUE;
        }

        return FALSE;
    };

    SetConsoleCtrlHandler(handler, TRUE);
}

inline void
catchDefaultOsSignals() {
    catchWin32Signals();
}
///////////////////////////////////////////////////////////////////////////////
#endif // Q_OS_UNIX
#endif // UNIX_CATCHER_HPP
///////////////////////////////////////////////////////////////////////////////
