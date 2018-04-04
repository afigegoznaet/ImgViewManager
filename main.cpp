#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QThread>


int main(int argc, char *argv[]){
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName("Custom Customer");
	QCoreApplication::setApplicationName("Clipart Viewer");

	QPixmap pixmap(":/Images/splash.png");
	//qDebug()<<pixmap.height();
//#if !defined(QT_DEBUG) || !defined(_WIN32)
	QSplashScreen splash(pixmap);
	QFont font = QApplication::font("QMenu");
	font.setStyleHint(QFont::Monospace);
	splash.setFont(font);
	splash.show();
#ifdef VALIDATE_LICENSE

	if (sodium_init() < 0){
		splash.showMessage("Unable to initialize libsodium", Qt::AlignCenter, Qt::red);
		QTextStream err(stderr);
		err << "Unable to initialize libsodium\n";
		err.flush();
		exit(-1);
	}

#endif
	//qApp->processEvents();
//#endif;
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
#else
	QObject::connect(&w, &MainWindow::splashText, [&](const QString& , int , const QColor &){
		locker.lock();
		//qDebug()<<message;
		locker.unlock();
	});
#endif
	w.setWindowTitle("Clipart Viewer");
	if(w.init()<0)
		return -1;
	//w.show();
	QTimer *timer = new QTimer();
	QObject::connect(timer, &QTimer::timeout, [&](){splash.finish(&w); w.show(); timer->deleteLater();});
	timer->start(2000);

	qDebug() << "Qt version: " << QT_VERSION_STR;
	return a.exec();
}
