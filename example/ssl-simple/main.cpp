#include "qhttp/qhttpserver.hpp"
#include "qhttp/qhttpserverresponse.hpp"
#include "qhttp/qhttpserverrequest.hpp"
#include "qhttp/qhttpsslconfig.hpp"

#include "../include/unixcatcher.hpp"

#include <QFile>

using namespace qhttp::server;
///////////////////////////////////////////////////////////////////////////////
namespace {
///////////////////////////////////////////////////////////////////////////////
int
usage(int, char* argv[]) {
    qDebug("invalid argument, usage:\n $>%s port_number", argv[0]);
    return -1;
}

void
dumpRequest(const qhttp::server::QHttpRequest* req) {
    qDebug(
        "\n--> %s : %s",
        qhttp::Stringify::toString(req->method()),
        req->url().toString().toUtf8().constData());

    req->headers().forEach([](auto cit) {
        qDebug(" %s: %s", cit.key().constData(), cit.value().constData());
    });
}

void
dumpCertificate(const char* appName) {
    auto certFileName = QString("%1-cer.pem").arg(appName);
    QFile ofile(certFileName);
    if (ofile.open(QFile::WriteOnly)) {
        QFile infile(":/cert");
        if (infile.open(QFile::ReadOnly))
            ofile.write(infile.readAll());
    }
}
///////////////////////////////////////////////////////////////////////////////
} // namespace anon
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv) {
    if (argc != 2)
        return usage(argc, argv);

    QCoreApplication app(argc, argv);
    catchDefaultOsSignals();
    dumpCertificate(argv[0]);

    QHttpServer server{&app};
    server.setSslConfig(qhttp::ssl::Config{":/key", ":/cert"});

    server.listen(argv[1], [](QHttpRequest* req, QHttpResponse* resp) {
        resp->setStatusCode(qhttp::ESTATUS_OK);
        resp->addHeader("Connection", "Close");
        resp->end("Hello TLS/SSL world\n");

        dumpRequest(req);
    });

    if (!server.isListening()) {
        qDebug("failed to listen on port: %s\n", argv[1]);
        return -1;
    }

    return app.exec();
}

