#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QThread>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("Custom Customer");
	QCoreApplication::setApplicationName("Clipart Viewer");



	QPixmap pixmap(":/Images/splash.png");
	qDebug()<<pixmap.height();
	QSplashScreen splash(pixmap);
	splash.show();
	QThread::sleep(2);

	MainWindow w;
	w.setWindowTitle("Clipart Viewer");

	splash.finish(&w);
	w.show();
	return a.exec();
}
