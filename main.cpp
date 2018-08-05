#include "MainWindow.hpp"
#include <QApplication>
#include <QSplashScreen>
#include <QThread>

#define ORG_NAME "Clipart Crew"
#define APP_NAME "Tropical Clipart 2018"
#define WINDOW_TITLE APP_NAME

int main(int argc, char *argv[]){
	QApplication a(argc, argv);
	QCoreApplication::setOrganizationName(ORG_NAME);
	QCoreApplication::setApplicationName(APP_NAME);

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
	QMutex splashLocker;

#if !defined(QT_DEBUG) && defined(_WIN32)
	QObject::connect(&w, &MainWindow::splashText, [&](const QString& message, int alignment, const QColor &color){
		QMutexLocker locker(&splashLocker);
		splash.showMessage(message, alignment, color);
	});
#else

	QObject::connect(&w, &MainWindow::splashText, [&](const QString& , int , const QColor &){
		splashLocker.lock();
		//qDebug()<<message;
		splashLocker.unlock();
	});
#endif
	w.init();
	w.setWindowTitle(WINDOW_TITLE);

	QTimer *timer = new QTimer();
	QObject::connect(timer, &QTimer::timeout, [&](){
		splashLocker.lock();
		splash.finish(&w);
		w.show();
		timer->deleteLater();
		splashLocker.unlock();
#ifdef _WIN32
		w.initProgressTaskbar();
#endif
	});
	timer->start(2000);

	qDebug() << "Qt version: " << QT_VERSION_STR;
	return a.exec();
}
