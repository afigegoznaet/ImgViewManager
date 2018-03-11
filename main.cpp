#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QThread>

int main(int argc, char *argv[]){
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("Custom Customer");
	QCoreApplication::setApplicationName("Clipart Viewer");

	QPixmap pixmap(":/Images/splash.png");
	qDebug()<<pixmap.height();
	QSplashScreen splash(pixmap);
	QFont font = QApplication::font("QMenu");
	font.setStyleHint(QFont::Monospace);
	//font.setPointSize(font.pointSize()*2);
	splash.setFont(font);
	splash.show();
	qApp->processEvents();

	qDebug()<<argc;
	for(int i=0; i<argc; i++)
		qDebug()<<argv[i];

	QString params = "";
	if(argc>1)
		params = QString(argv[1]);
	MainWindow w(params);
	QMutex locker;
#if !defined(QT_DEBUG) || !defined(_WIN32)
	QObject::connect(&w, &MainWindow::splashText, [&](const QString& message, int alignment, const QColor &color){
		locker.lock();
		splash.showMessage(message, alignment, color);
		locker.unlock();
	});
#endif
	w.setWindowTitle("Clipart Viewer");
	w.init();
	qApp->processEvents();

	splash.finish(&w);
	w.show();
	return a.exec();
}
