#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include "../include/gason.hpp"
#include "../include/jsonbuilder.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QLocalServer>
#include <QBasicTimer>
#include <QDateTime>

#include <signal.h>
#include <unistd.h>
///////////////////////////////////////////////////////////////////////////////
using namespace qhttp::server;

class Application : public QCoreApplication
{
public:
    explicit    Application(int& argc, char** argv) : QCoreApplication(argc, argv), iserver(this) {
    }

    bool        initialize() {
        QString iportOrPath = "8080"; //default tcp port
        if ( arguments().size() >= 2 )
            iportOrPath = arguments().at(1);

        iserver.listen(iportOrPath, [this](QHttpRequest* req, QHttpResponse* res){
            req->collectData(512);

            req->onEnd([this, req, res](){
                iintervalConnections++;
                res->addHeader("connection", "close");

                // gason++ writes lots of \0 into source buffer. so we have to make a writeable copy.
                char buffer[512] = {0};
                const QByteArray& body = req->collectedData();
                strncpy(buffer, body.constData(), std::min(511, body.length()));

                gason::JsonValue        root;
                if ( gason::jsonParse(buffer, root, iallocator) == gason::JSON_PARSE_OK ) {
                    gason::JsonValue command   = root("command");
                    gason::JsonValue clientId  = root("clientId");
                    gason::JsonValue requestId = root("requestId");

                    bool ok = false;
                    if ( strncmp(command.toString(&ok), "request", 7) == 0  &&
                         clientId.isNumber()    &&    requestId.isNumber() ) {

                        memset(buffer, 0, 512);
                        gason::JSonBuilder doc(buffer, 511);
                        doc.startObject()
                                .addValue("command", "response")
                                .addValue("clientId", clientId.toInt(&ok))
                                .addValue("requestId", requestId.toInt(&ok) + 1)
                                .endObject();

                        res->addHeader("content-length", QByteArray::number((int)strlen(buffer)));
                        res->setStatusCode(qhttp::ESTATUS_OK);
                        res->end(QByteArray(buffer));
                        return;
                    }
                }

                res->setStatusCode(qhttp::ESTATUS_BAD_REQUEST);
                res->end("bad request: the json value is not present or invalid!\n");
            });

            if ( req->headers().keyHasValue("command", "quit" ) ) {
                puts("user sends a quit request.\nGoodbye.");
                quit();
            }
        });

        if ( !iserver.isListening() ) {
            printf("listening on %s has been failed!\n", qPrintable(iportOrPath));
            return false;
        }


        ihrTimer.start();
        iintervalTimer.start(10000, Qt::CoarseTimer, this);
        puts("\nDateTime,AveTps,miliSecond,Count,TotalCount");
        return true;
    }

protected:
    void        timerEvent(QTimerEvent *e) {
        if ( e->timerId() != iintervalTimer.timerId() )
            return;

        itotalConnections   += iintervalConnections;
        quint32 miliSec      = (quint32) ihrTimer.elapsed();
        float   aveTps       = float(iintervalConnections * 1000.0) / float(miliSec);
        QString dateTime     = QLocale::c().toString(
                                   QDateTime::currentDateTime(),
                                   "yyyy-MM-dd hh:mm:ss");

        printf("%s,%.1f,%u,%u,%llu\n", qPrintable(dateTime),
               aveTps, miliSec,
               iintervalConnections, itotalConnections
               );

        iintervalConnections = 0;
        ihrTimer.start();
    }

protected:
    QHttpServer     iserver;

    quint32         iintervalConnections = 0;
    quint64         itotalConnections = 0;
    QElapsedTimer   ihrTimer;
    QBasicTimer     iintervalTimer;

    gason::JsonAllocator iallocator;

};

///////////////////////////////////////////////////////////////////////////////
void catchUnixSignals(const std::vector<int>& quitSignals,
                      const std::vector<int>& ignoreSignals = std::vector<int>()) {

    auto handler = [](int sig) ->void {
        printf("\nquit the application (user request signal = %d).\n", sig);
        QCoreApplication::quit();
    };

    for ( int sig : ignoreSignals )
        signal(sig, SIG_IGN);

    for ( int sig : quitSignals )
        signal(sig, handler);
}

int main(int argc, char *argv[]) {
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});

    Application app(argc, argv);
    if ( app.initialize() )
        app.exec();
    return 0;
}


