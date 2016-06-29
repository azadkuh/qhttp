#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include <QCoreApplication>

#include <QTimer>
#include <QFile>
#include "../include/unixcatcher.hpp"
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv) {
    QString port = "8080";
    if ( argc == 2 ) {
        port = argv[1]; // override default port
    }

    QCoreApplication app(argc, argv);
#if defined(Q_OS_UNIX)
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
#endif

    using namespace qhttp::server;
    QHttpServer server(&app);
    server.listen(port, [](QHttpRequest* req, QHttpResponse* res) {
        req->collectData(8*1024*1024); // maximum 8MB of data for each post request
        // the better approach is to use req->onData(...)

        req->onEnd([req, res](){
            res->setStatusCode(qhttp::ESTATUS_OK);
            res->addHeader("connection", "close"); // optional header (added by default)

            int size = req->collectedData().size();
            auto message = [size]() -> QByteArray {
                if ( size == 0 )
                    return "Hello World!\n";

                char buffer[65] = {0};
                qsnprintf(buffer, 64, "Hello!\nyou've sent me %d bytes!\n", size);
                return buffer;
            };

            res->end(message());  // response body data

            if ( size > 0 ) { // dump the incoming data into a file
                QFile f("dump.bin");
                if ( f.open(QFile::WriteOnly) )
                    f.write(req->collectedData());
            }
        });
    });

    if ( !server.isListening() ) {
        qDebug("failed: listening at %s!\n", qPrintable(port));
        return -1;
    }

    return app.exec();
}
