#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_chRtBt_clicked();

	void on_addPortBt_clicked();

	void on_rmPortBt_clicked();

	void on_rootE_textChanged(const QString &arg1);

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
