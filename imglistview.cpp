#include "imglistview.h"

ImgListView::ImgListView(QWidget *parent) : QListView(parent){
	fsModel = new QFileSystemModel(this);
	auto rootIdx = fsModel->setRootPath(QDir::rootPath());
	fsModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);

	QStringList filter;
	filter << "*.png";
	filter << "*.jpeg";
	filter << "*.jpg";
	fsModel->setNameFilters(filter);
	fsModel->setNameFilterDisables(false);

	thumbnailPainter = new ImgThumbnailDelegate(this);
	thumbnailPainter->setModel(fsModel);
	setItemDelegate(thumbnailPainter);

	setModel(fsModel);
	setViewMode(IconMode);
	setResizeMode(Adjust);

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	qDebug()<<screenGeometry;
	qDebug()<<screen->physicalSize();
	int height = screenGeometry.height();
	int width = screenGeometry.width();
	qDebug()<<height;
	qDebug()<<width;
	setIconSize(QSize(height/8,height/8));
	setLayoutMode (QListView::Batched);
	setUniformItemSizes(true);
}

void ImgListView::changeDir(QString dir){
	qDebug()<<dir;
	fsModel->setRootPath(dir);
	setRootIndex(fsModel->index(dir));
}
