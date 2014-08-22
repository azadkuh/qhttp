#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QUrlQuery>

#include <QLocalServer>

#include "../include/unixcatcher.hpp"
///////////////////////////////////////////////////////////////////////////////

void    runServer(QCoreApplication& app, const QString& portOrPath) {
    using namespace qhttp::server;

    QHttpServer server(&app);
    // listening tcp port or Unix path
    server.listen(portOrPath, [](QHttpRequest* req, QHttpResponse* res) {

        res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
        res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
        res->end("Hello World!\n");                 // response body data

        // when "connection: close", the req and res will be deleted automatically.

        // optionally let the clients to shut down the server
        if ( req->headers().keyHasValue("command", "quit") ) {
            printf("a client sends a quit command.\nserver quits.\n");
            QCoreApplication::quit();
            return;
        }
    });

    if ( !server.isListening() ) {
        fprintf(stderr, "failed. can not listen at port %s!\n", qPrintable(portOrPath));
        return;
    }

    app.exec();                 // application's main event loop
}

///////////////////////////////////////////////////////////////////////////////

void    runClient(QCoreApplication& app, const QString& portOrPath) {
    using namespace qhttp::client;

    QUrl url;
    if ( portOrPath.toUShort() > 0 ) {
        url.setScheme("http");
        url.setHost("localhost");
        url.setPort(portOrPath.toUShort());

    } else {
        url = QUrl::fromLocalFile(portOrPath);
    }


    QHttpClient  client(&app);

    bool success = client.request(qhttp::EHTTP_GET, url, [](QHttpResponse* res) {
        // response handler, called when the HTTP headers of the response are ready
        res->collectData(128);
        // called when all data in HTTP response have been read.
        res->onEnd([res]() {
            // print the XML body of the response
            puts("\n[incoming response:]");
            puts(res->collectedData().constData());
            puts("\n\n");

            QCoreApplication::instance()->quit();
        });

        // just for fun! print headers:
        puts("\n[Headers:]");
        const qhttp::THeaderHash& hs = res->headers();
        for ( auto cit = hs.constBegin(); cit != hs.constEnd(); cit++) {
            printf("%s : %s\n", cit.key().constData(), cit.value().constData());
        }
        fflush(stdout);
    });

    if ( !success ) {
        fprintf(stderr, "can not send a request to %s\n", qPrintable(url.toString()));
        return;
    }

    app.exec();
}

///////////////////////////////////////////////////////////////////////////////

void    runWeatherClient(QCoreApplication& app) {
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
            puts("\n[incoming response: (body)]");
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
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});

    app.setApplicationName("helloworld");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("a HelloWorld example for http client and server.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("mode",
                                 "working mode: server, client or weather. default: server");

    parser.addOption(QCommandLineOption(QStringList() << "p" << "port",
                                        "tcp port number or path of UNIX socket in server/client modes, default: 8080",
                                        "port/path", "8080"));
    parser.process(app);


    QStringList posArgs = parser.positionalArguments();
    if ( posArgs.size() == 1    &&    posArgs.at(0) == QLatin1Literal("server") ) {
        runServer(app, parser.value("port"));

    } else if ( posArgs.size() == 1    &&    posArgs.at(0) == QLatin1Literal("client") ) {
        runClient(app, parser.value("port"));

    } else if ( posArgs.size() == 1    &&    posArgs.at(0) == QLatin1Literal("weather") ) {
        runWeatherClient(app);

    } else {
        parser.showHelp(0);
    }

    return 0;
}
