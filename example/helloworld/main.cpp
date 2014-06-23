#include "qhttpserver.hpp"
#include "qhttpresponse.hpp"
#include "qhttprequest.hpp"

#include <QtCore/QCoreApplication>

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);

    QHttpServer server(&app);
    server.listen(8080);            // port to listening on

    QObject::connect(&server, &QHttpServer::newRequest, [](QHttpRequest* req, QHttpResponse* res) {
        res->setHeader("connection", "close");      // set connection type to close (vs default keep-alive)
        res->writeHead(QHttpResponse::STATUS_OK);   // status 200
        res->end("Hello World!\n");                 // response body data

        res->deleteLater();                         // optional as connection type is "close"
        req->deleteLater();                         // optional as connection type is "close"
    });


    app.exec();                 // application's main event loop
}
