#include "imglistview.h"

ImgListView::ImgListView(QWidget *parent) : QListView(parent) , isExiting(false){
	fsModel = new QFileSystemModel(this);
	auto rootIdx = fsModel->setRootPath(QDir::rootPath());
	fsModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);

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
	connect(this, SIGNAL(callUpdate(const QModelIndex &)),
			this, SLOT(update(const QModelIndex &)), Qt::QueuedConnection);
	connect(this,SIGNAL(doubleClicked(QModelIndex)),
			this,SLOT(onDoubleClicked()));
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	selectionModel()->setModel(fsModel);
}

void ImgListView::changeDir(QString dir){
	prefetchProc.cancel();
	qDebug()<<"Changing dir";
	qDebug()<<dir;
	fsModel->setRootPath(dir);
	setRootIndex(fsModel->index(dir));
	prefetchProc = QtConcurrent::run([&](){prefetchThumbnails();});
	qDebug()<<"Prefetch started";
}

void ImgListView::prefetchThumbnails(){
	int i=0;
	auto flags = Qt::ColorOnly | Qt::ThresholdDither
			| Qt::ThresholdAlphaDither;
	for(auto& fileInfo : fsModel->rootDirectory().entryInfoList(filter)){

		//qDebug()<<"not exiting";
		i++;
		QSize iconSize = this->iconSize();
		QImageReader reader(fileInfo.absoluteFilePath());
		auto picSize = reader.size();
		double coef = picSize.height()*1.0/picSize.width();

		if(coef>1)
			iconSize.setWidth(iconSize.width()/coef);
		else
			iconSize.setHeight(iconSize.height()*coef);

		reader.setScaledSize(iconSize);
		reader.setAutoTransform(true);
		reader.setQuality(15);

		if(isExiting)
			return;

		auto pm = QPixmap::fromImageReader(&reader, flags);
		QPixmapCache::insert(fileInfo.absoluteFilePath(), pm);
		auto idx = fsModel->index(fileInfo.absoluteFilePath());

		auto appRegion = visibleRegion();
		//if(appRegion.contains(visualRect(idx)))
			emit callUpdate(idx);
	}
	qDebug()<<"Prefetch finished";
}

void ImgListView::keyPressEvent(QKeyEvent *event){
	auto key = event->key();

	if(key == Qt::Key_Return || key == Qt::Key_Enter)
		onDoubleClicked();
	else
		QAbstractItemView::keyPressEvent(event);
}


void ImgListView::onDoubleClicked(){
	auto selectedThumbnails = selectionModel()->selectedIndexes();
	for(auto &index : selectedThumbnails){
		QFileInfo info=fsModel->fileInfo(index);
		QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
	}

}

void ImgListView::prepareExit(){
	isExiting = true;
	prefetchProc.waitForFinished();
}
