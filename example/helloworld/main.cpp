#include "qhttpserver.hpp"
#include "qhttpresponse.hpp"
#include "qhttprequest.hpp"

#include <QtCore/QCoreApplication>

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);

    qhttp::server::QHttpServer server(&app);
    server.listen(8080);            // port to listening on

    QObject::connect(&server, &qhttp::server::QHttpServer::newRequest,
                     [](qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res) {
        res->setHeader("connection", "close");      // set connection type to close (vs default keep-alive)
        res->writeHead(qhttp::ESTATUS_OK);          // status 200
        res->end("Hello World!\n");                 // response body data

        // when connection closes, the req and res will be deleted automatically.
    });


    app.exec();                 // application's main event loop
}
