#ifndef UNIX_CATCHER_HPP
#define UNIX_CATCHER_HPP

#if defined(Q_OS_UNIX)
#include <QCoreApplication>
#include <signal.h>
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////
void catchUnixSignals(const std::vector<int>& quitSignals,
                      const std::vector<int>& ignoreSignals = std::vector<int>()) {

    auto handler = [](int sig) ->void {
        printf("\nquit the application (user request signal = %d).\n", sig);
        QCoreApplication::quit();
    };

    for ( int sig : ignoreSignals )
        signal(sig, SIG_IGN);

    for ( int sig : quitSignals )
        signal(sig, handler);
}
///////////////////////////////////////////////////////////////////////////////

#endif // Q_OS_UNIX

#endif // UNIX_CATCHER_HPP
