#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "qhttpserver.hpp"
#include "qhttpserverconnection.hpp"
#include "qhttpserverrequest.hpp"
#include "qhttpserverresponse.hpp"

#include <QtCore/QCoreApplication>
#include <QDateTime>
#include <QLocale>

#include "../include/unixcatcher.hpp"

using namespace qhttp::server;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);


	QString portOrUnixSocket("10022"); // default: TCP port 10022

	QHttpServer server(this);
	server.listen(portOrUnixSocket, [&](QHttpRequest* req, QHttpResponse* res)
	{

	});

	if ( !server.isListening() ) {
		fprintf(stderr, "can not listen on %s!\n", qPrintable(portOrUnixSocket));
		return ;
	}

}

MainWindow::~MainWindow()
{
	delete ui;
}
