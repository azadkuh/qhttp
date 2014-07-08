#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include <QtCore/QCoreApplication>

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);

    qhttp::server::QHttpServer server(&app);
    server.listen(8080);            // port to listening on

    QObject::connect(&server, &qhttp::server::QHttpServer::newRequest,
                     [](qhttp::server::QHttpRequest* req, qhttp::server::QHttpResponse* res) {

        res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
        res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
        res->end("Hello World!\n");                 // response body data

        // when "connection: close", the req and res will be deleted automatically.
    });


    app.exec();                 // application's main event loop
}
