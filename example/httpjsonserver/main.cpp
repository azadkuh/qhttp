#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QtCore/QDateTime>

#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include "../include/gason.hpp"
#include "../include/jsonbuilder.hpp"

///////////////////////////////////////////////////////////////////////////////

class ClientHandler : public QObject
{
public:
    explicit        ClientHandler(qhttp::server::QHttpRequest*  req,
                                     qhttp::server::QHttpResponse* res) : QObject(res) {

        QObject::connect(req, &qhttp::server::QHttpRequest::data,
                         [this, req](const QByteArray& chunk) {
            // data attack!
            if ( ibody.size() > 4096 )
                req->connection()->close();
            else
                ibody.append(chunk);
        });

        QObject::connect(req, &qhttp::server::QHttpRequest::end, [this, req, res](){
            res->addHeader("connection", "close");

            // gason++ writes lots of \0 into source buffer. so we have to make a writeable copy.
            char buffer[4907] = {0};
            strncpy(buffer, ibody.constData(), 4096);

            gason::JsonAllocator    allocator;
            gason::JsonValue        root;

            bool  clientStatus = false;

            if ( gason::jsonParse(buffer, root, allocator) == gason::JSON_PARSE_OK ) {
                gason::JsonValue command   = root("command");
                gason::JsonValue clientId  = root("clientId");
                gason::JsonValue requestId = root("requestId");

                bool ok = false;
                if ( strncmp(command.toString(&ok), "request", 7) == 0  &&
                     clientId.isNumber()    &&    requestId.isNumber() ) {

                    memset(buffer, 0, 4096);
                    gason::JSonBuilder doc(buffer, 4096);
                    doc.startObject()
                            .addValue("command", "response")
                            .addValue("clientId", clientId.toInt(&ok))
                            .addValue("requestId", requestId.toInt(&ok) + 1)
                            .endObject();

                    res->addHeader("content-length", QByteArray::number((int)strlen(buffer)));

                    clientStatus = true;
                }
            }

            if ( clientStatus ) {
                res->setStatusCode(qhttp::ESTATUS_OK);
                res->end(QByteArray(buffer));

            } else {
                res->setStatusCode(qhttp::ESTATUS_BAD_REQUEST);
                res->end("bad request: the json value is not present or invalid!\n");
            }

            if ( req->headers().keyHasValue("command", "quit" ) ) {
                puts("a quit header is received!");

                QCoreApplication::instance()->quit();
            }
        });
    }

protected:
    QByteArray      ibody;
};

///////////////////////////////////////////////////////////////////////////////

class Application : public QCoreApplication
{
public:
    explicit    Application(int& argc, char** argv)
        : QCoreApplication(argc, argv), iserver(this) {
        setApplicationName("qtjsonserver");
        setApplicationVersion("1.0.0");

        QCommandLineParser parser;
        parser.setApplicationDescription("a simple http server for responding incomming JSon.\n" \
                                         "by Amir Zamani.");
        parser.addHelpOption();
        parser.addVersionOption();
        parser.addPositionalArgument("port",
                                     "listening port. default: 5533");

        parser.addOption(QCommandLineOption(QStringList() << "t" << "timeout",
                                            "maximum timeout for an open connection. default: 5000",
                                            "number", "5000"));

        parser.process(*this);

        iport                  = 5533;
        itimeOut               = parser.value("timeout").toUInt();
        iserver.setTimeOut(itimeOut);

        QStringList args = parser.positionalArguments();
        if ( args.size() >= 1 ) {
            iport = args.at(0).toUShort();
        }

        printf("listening @ %d --timeout %u\n\n",
               iport, itimeOut);
    }

    bool        start() {
        using namespace qhttp::server;
        QObject::connect(&iserver, &QHttpServer::newRequest,
                         [this](QHttpRequest* req, QHttpResponse* res){
            new ClientHandler(req, res);
        });

        if ( !iserver.listen(QHostAddress::Any, iport) ) {
            printf("can not listen on port #%d.\n", iport);
            return false;
        }

        return true;
    }

protected:
    qhttp::server::QHttpServer  iserver;

    quint16         iport;
    size_t          imaxPendingConnections;
    quint32         itimeOut;
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    Application app(argc, argv);
    if ( app.start() )
        app.exec();

    puts("\n\nend of http server.\n\n");
    return 0;
}


