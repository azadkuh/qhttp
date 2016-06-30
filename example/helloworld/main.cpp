#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

#include "qhttpclient.hpp"
#include "qhttpclientrequest.hpp"
#include "qhttpclientresponse.hpp"

#include "../include/unixcatcher.hpp"

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QUrlQuery>

#include <QLocalServer>
#include <QTimer>
#include <QFile>
///////////////////////////////////////////////////////////////////////////////
namespace {
///////////////////////////////////////////////////////////////////////////////

void runServer(const QString& portOrPath) {
    using namespace qhttp::server;

    QHttpServer server(qApp);
    // listening tcp port or Unix path
    server.listen(portOrPath, [](QHttpRequest* req, QHttpResponse* res) {
        req->collectData();

        req->onEnd([req, res](){
            res->setStatusCode(qhttp::ESTATUS_OK); // status 200
            res->addHeader("connection", "close"); // optional(default) header

            int size = req->collectedData().size();
            auto message = [size]() -> QByteArray {
                if ( size == 0 )
                    return "Hello World!\n";

                char buffer[65] = {0};
                qsnprintf(buffer, 64, "Hello!\nyou've sent me %d bytes!\n", size);
                return buffer;
            };

            res->end(message());  // reponse body data
        });

        const auto& h = req->headers();
        // optionally let the clients to shut down the server
        if ( h.keyHasValue("command", "quit") ) {
            printf("a client sends a quit command.\nserver quits.\n");
            QCoreApplication::quit();
            return;
        }

        // just for fun! print meta information:
        qDebug("\n--> %s : %s",
                qhttp::Stringify::toString(req->method()),
                qPrintable(req->url().toString().toUtf8())
              );
        qDebug("[Headers (%d)]", h.size());
        h.forEach([](auto iter) {
            qDebug(" %s : %s",
                    iter.key().constData(),
                    iter.value().constData()
                  );
        });
    });

    if ( !server.isListening() ) {
        fprintf(stderr, "failed. can not listen at port %s!\n", qPrintable(portOrPath));
        return;
    }

    qApp->exec(); // application's main event loop
}

#if defined(QHTTP_HAS_CLIENT)

void runClient(QString url) {
    using namespace qhttp::client;
    // ensure there the url has http://
    if ( !url.startsWith("http://") && !url.startsWith("https://") )
        url.prepend("http://");

    QHttpClient  client(qApp);

    bool success = client.request(qhttp::EHTTP_GET, url, [](QHttpResponse* res) {
        // response handler, called when the HTTP headers of the response are ready
        res->collectData();
        // called when all data in HTTP response have been read.
        res->onEnd([res]() {
            // print the XML body of the response
            qDebug("\nreceived %d bytes of http body:\n%s\n",
                    res->collectedData().size(),
                    res->collectedData().constData()
                  );

            qApp->quit();
        });

        // just for fun! print headers:
        qDebug("\n[Headers:]");
        res->headers().forEach([](auto cit) {
            qDebug("%s : %s", cit.key().constData(), cit.value().constData());
        });
    });

    if ( !success ) {
        qDebug("failed: can not send a request to %s\n", qPrintable(url));
        return;
    }

    qApp->exec();
}

void runWeatherClient(const QString& cityName) {
    using namespace qhttp::client;

    QHttpClient client(qApp);
    QByteArray  httpBody;

    QUrl weatherUrl(QString("http://wttr.in/%1").arg(cityName));

    client.request(qhttp::EHTTP_GET, weatherUrl, [&httpBody](QHttpResponse* res) {
        // response handler, called when the HTTP headers of the response are ready

        // gather HTTP response data manually
        res->onData([&httpBody](const QByteArray& chunk) {
            httpBody.append(chunk);
        });

        // called when all data in HTTP response have been read.
        res->onEnd([&]() {
            // print the XML body of the response
            qDebug(" %d bytes of body's been written in weather.html\n", httpBody.size());
            QFile f("weather.html");
            if ( f.open(QFile::WriteOnly) )
                f.write(httpBody);

            qApp->quit();
        });


        // just for fun! print headers:
        qDebug("\n[Headers:]");
        res->headers().forEach([](auto cit) {
            qDebug("%s : %s", cit.key().constData(), cit.value().constData());
        });
    });

    // set a timeout for making the request
    client.setConnectingTimeOut(10000, []{
        qDebug("connecting to HTTP server timed out!");
        qApp->quit();
    });

    qApp->exec();
}


#endif // QHTTP_HAS_CLIENT

///////////////////////////////////////////////////////////////////////////////
} // namespace anon
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);
#if defined(Q_OS_UNIX)
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
#endif

    app.setApplicationName("helloworld");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("a HelloWorld example for http client and server.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("mode",
            "working mode: server, client or weather. default: server");

    parser.addOption({
            {"l", "listen"},
            "listening tcp port number in server mode (default 8080)",
            "portNo", "8080"});
    parser.addOption({
            {"u", "url"},
            "fetch url data in client mode",
            "address", "http://www.google.com"});
    parser.addOption({
            {"g", "geolocation"},
            "a city name [,country name] in weather mode, default: Tehran",
            "city", "Tehran"});
    parser.process(app);


    QStringList posArgs = parser.positionalArguments();
    if ( posArgs.size() != 1 ) {
        parser.showHelp(0);

    } else {
        const auto& mode = posArgs.at(0);

        if ( mode == QLatin1Literal("server") )
            runServer(parser.value("listen"));

#if defined(QHTTP_HAS_CLIENT)
        else if ( mode == QLatin1Literal("client") )
            runClient(parser.value("url"));

        else if ( mode == QLatin1Literal("weather") )
            runWeatherClient(parser.value("geolocation"));
#else
        else if ( mode == QLatin1Literal("client")
                || mode == QLatin1Literal("weather") )
            qDebug("qhttp::client has not been enabled at build time");
#endif // QHTTP_HAS_CLIENT
    }

    return 0;
}
