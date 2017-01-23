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

#include <QFile>
#include <QFileDialog>

#include <map>
#include <fstream>

using namespace qhttp::server;
using namespace std;

#define Y2X(X)	#X
#define MAC2STR(Y) Y2X(Y)
#define SITEROOT .


map<string,string> mime;

QString root=".";
QFileDialog *getD;
map<QString, QHttpServer> servers;

void fileProvider(QHttpRequest* req, QHttpResponse* res)
{
	QString docname=root+(req->url().toString()==("/") ?("/index.html"):req->url().toString()) ;
	QFile doc(docname);

	if(not doc.open(QFile::ReadOnly))
	{
		QByteArray body = "Sorry Not found";
		res->addHeader("Content-Length", QString::number(body.size()).toUtf8());
		res->setStatusCode(qhttp::TStatusCode::ESTATUS_NOT_FOUND);
		res->write(body);
		return;
	}

	try {
		auto nm=docname.toStdString();
		auto ext=nm.substr(nm.find_last_of('.'));
		res->addHeader("Content-Type",mime[ext].data());
	} catch (...) {
		res->addHeader("Content-Type","application/octet-stream");
	}

	res->addHeader("Content-Length", QString::number(doc.size()).toUtf8());
	res->addHeader("Access-Control-Allow-Origin"," * ");
	res->addHeader("Access-Control-Allow-Headers","GET,POST,PUT");
	res->setStatusCode(qhttp::TStatusCode::ESTATUS_OK);
	res->write(doc.readAll());
}



MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	getD=new QFileDialog();
	getD->setFileMode(QFileDialog::Directory);
	getD->setOption(QFileDialog::ShowDirsOnly);

	ui->rootE->setText(root);

	if(mime.empty())
	{
		QFile fk(":/mime_types.txt");
		fk.open(QIODevice::ReadOnly | QIODevice::Text);

		do
		{
			QString mime_kv_pair;
			mime_kv_pair=fk.readLine();
			auto kv=mime_kv_pair.split("\t");
			mime[kv[0].toStdString()]=kv[1].toStdString();
		}while(fk.canReadLine());
	}

	this->on_addPortBt_clicked();
}

MainWindow::~MainWindow()
{
	delete ui;
}




void MainWindow::on_chRtBt_clicked()
{
	if(getD->exec())
		ui->rootE->setText(root=getD->selectedFiles()[0]);
}

void MainWindow::on_addPortBt_clicked()
{
	auto np=ui->portE->text();
	if(servers.find(np)==servers.end())
	{
		servers[np].listen(np,fileProvider);
		if(servers[np].isListening())
			ui->portsE->addItem(np);
		else
			servers.erase(np);
	}
}

void MainWindow::on_rmPortBt_clicked()
{
	if(ui->portsE->count()>0)
	{
		servers.erase(ui->portsE->currentItem()->text());
		ui->portsE->takeItem(ui->portsE->currentRow());
	}
}

void MainWindow::on_rootE_textChanged(const QString &nroot)
{
	root=nroot;
}
