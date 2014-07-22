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
    // listening on 0.0.0.0:8080
    server.listen(QHostAddress::Any, 8080, [](QHttpRequest* req, QHttpResponse* res) {

        res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
        res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
        res->end("Hello World!\n");                 // response body data

        // when "connection: close", the req and res will be deleted automatically.
    });

    if ( !server.isListening() ) {
        fprintf(stderr, "failed. can not listen at port 8080!\n");
        return;
    }

    app.exec();                 // application's main event loop
}

void    runClient(QCoreApplication& app) {
    using namespace qhttp::client;

    QHttpClient  client(&app);
    QByteArray   httpBody;

    QUrl weatherUrl("http://api.openweathermap.org/data/2.5/weather?q=tehran,ir&units=metric&mode=xml");

    client.request(qhttp::EHTTP_GET, weatherUrl, [&httpBody](QHttpResponse* res) {
        // response handler, called when the HTTP headers of the response are ready

        // gather HTTP response data
        res->onData([&httpBody](const QByteArray& chunk) {
            httpBody.append(chunk);
        });

        // called when all data in HTTP response have been read.
        res->onEnd([&httpBody]() {
            // print the XML body of the response
            puts("\n[incoming response:]");
            puts(httpBody.constData());
            puts("\n\n");

            QCoreApplication::instance()->quit();
        });

        // just for fun! print headers:
        puts("\n[Headers:]");
        const qhttp::THeaderHash& hs = res->headers();
        for ( auto cit = hs.constBegin(); cit != hs.constEnd(); cit++) {
            printf("%s : %s\n", cit.key().constData(), cit.value().constData());
        }
    });


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
