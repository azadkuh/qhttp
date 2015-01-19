#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QLocalServer>
#include <QBasicTimer>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "../include/unixcatcher.hpp"
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

                const QByteArray& body = req->collectedData();
                QJsonObject root       = QJsonDocument::fromJson(body).object();
                if ( !root.isEmpty()  && root.contains("command")  &&
                     root.contains("clientId")  && root.contains("requestId") ) {

                    root["command"]     = QLatin1Literal("response");
                    root["requestId"]   = root["requestId"].toInt() + 1;
                    QByteArray doc      = QJsonDocument(root).toJson(QJsonDocument::Compact);

                    res->addHeader("content-length", QByteArray::number((int)strlen(doc)));
                    res->setStatusCode(qhttp::ESTATUS_OK);
                    res->end(doc);
                    return;

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
};

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
    Application app(argc, argv);
#if defined(Q_OS_UNIX)
    catchUnixSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
#endif

    if ( app.initialize() )
        app.exec();

    return 0;
}


