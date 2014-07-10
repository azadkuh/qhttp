#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"

#include <QtCore/QCoreApplication>
#include <QUrlQuery>
///////////////////////////////////////////////////////////////////////////////
void    runServer(QCoreApplication& app) {
    using namespace qhttp::server;

    QHttpServer server(&app);
    // port to listening on
    if ( !server.listen(8080) ) {
        fprintf(stderr, "failed. can not listen at port 8080!\n");
        return;
    }

    QObject::connect(&server, &QHttpServer::newRequest,
                     [](QHttpRequest* req, QHttpResponse* res) {

        res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
        res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
        res->end("Hello World!\n");                 // response body data

        // when "connection: close", the req and res will be deleted automatically.
    });


    app.exec();                 // application's main event loop
}

void    runClient(QCoreApplication& app) {
    using namespace qhttp::client;

    QByteArray  buffer;
    QHttpClient client(&app);

    QObject::connect(&client, &QHttpClient::httpConnected, [](QHttpRequest* req){
        req->addHeader("connection", "close");
        req->addHeader("cache-control", "no-cache");
        req->end();
    });

    QObject::connect(&client, &QHttpClient::newResponse, [&](QHttpResponse* res){

        QObject::connect(res, &QHttpResponse::data, [&buffer](const QByteArray& chunk){
            buffer.append(chunk);
        });

        QObject::connect(res, &QHttpResponse::end, [&buffer](){
            puts("\n[incoming response:]");
            puts(buffer.constData());
            puts("\n\n");

            QCoreApplication::instance()->quit();
        });

        puts("\n[Headers:]");
        for ( auto cit = res->headers().constBegin(); cit != res->headers().constEnd(); cit++) {
            printf("%s : %s\n",
                   cit.key().constData(),
                   cit.value().constData());
        }
    });

    QUrl url("http://api.openweathermap.org/data/2.5/weather?q=tehran,ir&units=metric&mode=xml");
    client.request(qhttp::EHTTP_GET, url);

    app.exec();
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);

    const QStringList& args = app.arguments();
    if ( args.size() == 2    &&    args.at(1) == "client" )
        runClient(app);
    else
        runServer(app);

    return 0;
}
