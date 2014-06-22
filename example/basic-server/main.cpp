#include "httpserver.hpp"

#include <QtCore/QCoreApplication>



int main(int argc, char ** argv) {
    QCoreApplication app(argc, argv);

    am::HttpServer server(&app);
    server.listen(8080);

    QObject::connect(
                &server,  SIGNAL(quit()),
                &app,     SLOT(quit())
                );

    app.exec();
}
