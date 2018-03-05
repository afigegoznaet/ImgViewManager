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
	MainWindow w;
	QMutex locker;
	QObject::connect(&w, &MainWindow::splashText, [&](const QString& message, int alignment, const QColor &color){
		locker.lock();
		splash.showMessage(message, alignment, color);
		locker.unlock();
	});

	w.setWindowTitle("Clipart Viewer");
	w.init();
	qApp->processEvents();

	splash.finish(&w);
	w.show();
	return a.exec();
}
