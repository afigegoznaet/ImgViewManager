#include "imglistview.h"

ImgListView::ImgListView(QWidget *parent) : QListView(parent) , isExiting(false){
	QFileSystemModel *fsModel = new QFileSystemModel(this);
	auto rootIdx = fsModel->setRootPath(QDir::rootPath());
	fsModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);

	namedFilters << "*.png";
	namedFilters << "*.jpeg";
	namedFilters << "*.jpg";

	filterText = "";

	fsModel->setNameFilterDisables(false);
	proxyModel= new ThumbnailsFileModel(this);
	proxyModel->setSourceModel(fsModel);
	proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	setModel(proxyModel);


	applyFilter("");
	thumbnailPainter = new ImgThumbnailDelegate(this);
	thumbnailPainter->setModel(proxyModel);
	setItemDelegate(thumbnailPainter);

	//setModel(fsModel);
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
	selectionModel()->setModel(proxyModel);
	setMovement(Movement::Static);
	connect(fsModel, &QFileSystemModel::directoryLoaded, [&](){
		emit numFiles(
					proxyModel->rootDirectory().entryInfoList().count(),
					proxyModel->rowCount(rootIndex())
					);
	} );
}

void ImgListView::changeDir(QString dir){
	prefetchProc.cancel();
	qDebug()<<"Changing dir";
	qDebug()<<dir;
	proxyModel->setRootPath(dir);
	applyFilter("");
	setRootIndex(proxyModel->index(dir));
	//prefetchProc =
	QtConcurrent::run([&](){prefetchThumbnails();});
	qDebug()<<"Prefetch started";
}

void ImgListView::prefetchThumbnails(){
	int i=0;
	auto flags = Qt::ColorOnly | Qt::ThresholdDither
			| Qt::ThresholdAlphaDither;
	for(auto& fileInfo : proxyModel->rootDirectory().entryInfoList(namedFilters)){

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
		auto idx = proxyModel->index(fileInfo.absoluteFilePath());

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
		QFileInfo info=proxyModel->fileInfo(index);
		QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
	}

}

void ImgListView::prepareExit(){
	isExiting = true;
	prefetchProc.waitForFinished();
}

void ImgListView::applyFilter(QString filter){
	if(filter.length()<1)
		proxyModel->setFilterWildcard("*.*");

	filterText = filter;
	proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive,
												QRegExp::FixedString));

	emit numFiles(
				proxyModel->rootDirectory().entryInfoList(namedFilters).count(),
				proxyModel->rowCount(rootIndex())
				);
}
