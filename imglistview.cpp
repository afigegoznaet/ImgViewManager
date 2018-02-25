#include "imglistview.h"

ImgListView::ImgListView(QWidget *parent) : QListView(parent) , isExiting(false){
	QFileSystemModel *fsModel = new QFileSystemModel(this);
	auto rootIdx = fsModel->setRootPath(QDir::rootPath());
	fsModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);

	namedFilters << "*.png";
	namedFilters << "*.jpeg";
	namedFilters << "*.jpg";

	filterText = "";

	fsModel->setNameFilters(namedFilters);
	fsModel->setNameFilterDisables(false);
	proxyModel= new ThumbnailsFileModel(this);
	proxyModel->setSourceModel(fsModel);
	proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	setModel(proxyModel);


	applyFilter("");
	thumbnailPainter = new ImgThumbnailDelegate(thumbnailsCache, this);
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
	//thumbnailPainter->prepareExit();
	prefetchProc.cancel();
	thumbnailsCache.clear();
	prefetchProc = QtConcurrent::run([&](){prefetchThumbnails();});
	qDebug()<<"Prefetch started";
}

void ImgListView::prefetchThumbnails(){
	int i=0;
	auto flags = Qt::ColorOnly | Qt::ThresholdDither
			| Qt::ThresholdAlphaDither;
	QString fileName = proxyModel->rootDirectory().absolutePath();
	fileName +="/.thumbnails";
	QFile thumbnailsFile(fileName);


	QDataStream in (&thumbnailsFile);
	in.setVersion(QDataStream::Qt_5_7);
	if(!thumbnailsFile.open(QIODevice::ReadOnly))
		qDebug()<<"Error opening thumbnails file";
	else
		in>> thumbnailsCache;

	thumbnailsFile.close();

	qDebug()<<sizeof(thumbnailsCache);
	int countAtStart = thumbnailsCache.count();

	for(auto& fileInfo : proxyModel->rootDirectory().entryInfoList(namedFilters)){
		QPixmap cachedImage;
		auto currentFileName = fileInfo.fileName();
		if(thumbnailsCache.contains(currentFileName)){
			cachedImage = thumbnailsCache[currentFileName];
		}else{
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
			cachedImage = QPixmap::fromImageReader(&reader);
			thumbnailsCache.insert(currentFileName,cachedImage);
		}

		if(isExiting)
			return;

		auto idx = proxyModel->index(fileInfo.absoluteFilePath());
		emit callUpdate(idx);
	}

	if(countAtStart != thumbnailsCache.count()){
		qDebug()<<"Saving cache to file";
		thumbnailsFile.open(QIODevice::WriteOnly);
		thumbnailsFile.resize(0);
		QDataStream out (&thumbnailsFile);
		out.setVersion(QDataStream::Qt_5_7);
		out<<thumbnailsCache;
		thumbnailsFile.flush();
		thumbnailsFile.close();

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
	thumbnailPainter->prepareExit();
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
