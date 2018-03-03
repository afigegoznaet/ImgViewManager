#include "imglistview.h"

ImgListView::ImgListView(QWidget *parent) : QListView(parent), stopPrefetching(false){
	QFileSystemModel *fsModel = new QFileSystemModel(this);
	fsModel->setRootPath(QDir::rootPath());
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
	setGridSize(QSize(iconSize().width()+32, iconSize().height()+32));
	qDebug()<<"Icon size: "<<iconSize();
	qDebug()<<"Grid size: "<<gridSize();
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

	copyDialog = new ProgressDialog(this);

	connect(this, SIGNAL(setFileAction(QFileInfoList,QString)),
			copyDialog, SLOT(processFileAction(QFileInfoList,QString)));
	connect(this, SIGNAL(callFullUpdate()), this, SLOT(update()), Qt::QueuedConnection);
}

void ImgListView::changeDir(QString dir){
	stopPrefetching = true;

	//qDebug()<<"Changing dir";
	//qDebug()<<dir;
	proxyModel->setRootPath(dir);
	//applyFilter("");
	setRootIndex(proxyModel->fileIndex(dir));

	//prefetchProc.cancel();
	prefetchProc.waitForFinished();
	stopPrefetching = false;
	thumbnailPainter->stopDrawing();
	thumbnailsCache.clear();
	thumbnailPainter->resumeDrawing();
	prefetchProc = QtConcurrent::run([&](){prefetchThumbnails();});
	emit callFullUpdate();

}

void ImgListView::prefetchThumbnails(){
	//auto flags = Qt::ColorOnly | Qt::ThresholdDither
			//| Qt::ThresholdAlphaDither;
	QString fileName = proxyModel->rootDirectory().absolutePath();
	fileName +="/.kthumbnails";
	QFile thumbnailsFile(fileName);

	QDataStream in (&thumbnailsFile);
	in.setVersion(QDataStream::Qt_5_7);
	if(thumbnailsFile.open(QIODevice::ReadOnly))
		in>> thumbnailsCache;

	thumbnailsFile.close();

	//qDebug()<<"Sizeof: "<<sizeof(thumbnailsCache);
	int countAtStart = thumbnailsCache.count();

	for(auto& fileInfo : proxyModel->rootDirectory().entryInfoList(namedFilters)){
		if(stopPrefetching)
			break;
		auto currentFileName = fileInfo.fileName();
		if(!thumbnailsCache.contains(currentFileName)) {
			QSize iconSize = this->iconSize();
			QImageReader reader(fileInfo.absoluteFilePath());
			auto picSize = reader.size();
			if(picSize.width()>iconSize.width() || picSize.height()>iconSize.height()){
				auto picSize = reader.size();
				double coef = picSize.height()*1.0/picSize.width();
				if(coef>1)
					iconSize.setWidth(iconSize.width()/coef);
				else
					iconSize.setHeight(iconSize.height()*coef);
			}
			reader.setScaledSize(iconSize);
			reader.setAutoTransform(true);
			reader.setQuality(15);
			thumbnailPainter->stopDrawing();
			thumbnailsCache.insert(currentFileName, QPixmap::fromImageReader(&reader));
			thumbnailPainter->resumeDrawing();
		}

		QPersistentModelIndex idx = proxyModel->fileIndex(fileInfo.absoluteFilePath());
		emit callUpdate(idx);
	}

	if(countAtStart != thumbnailsCache.count()){
		//qDebug()<<"Saving cache to file";
		if(thumbnailsFile.open(QIODevice::WriteOnly)){
			thumbnailsFile.resize(0);
			QDataStream out (&thumbnailsFile);
			out.setVersion(QDataStream::Qt_5_7);
			out<<thumbnailsCache;
			thumbnailsFile.flush();
			thumbnailsFile.close();
		}


	}

	//qDebug()<<"Prefetch finished";
	emit callFullUpdate();
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
	stopPrefetching = true;
	thumbnailPainter->stopDrawing();
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


void ImgListView::exportImages(){
	auto selections = selectionModel()->selectedIndexes();
	if( 0 == selections.count() ){
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setWindowTitle("Warning");
		msgBox.setText("No files selected");
		//msgBox.setInformativeText("Do you want to save your changes?");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		return;
	}
	QFileDialog selector(this,"Select output folder",
						 proxyModel->rootDirectory().absolutePath());
	selector.setFileMode(QFileDialog::DirectoryOnly);
	if (!selector.exec())
		return;
	else
		selector.selectedFiles();

	QString expDir = selector.selectedFiles().first();

	if(0 == expDir.compare(proxyModel->rootDirectory().absolutePath())){
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setWindowTitle("Wrong directory");
		msgBox.setText("Output directory can not be the same");
		//msgBox.setInformativeText("Do you want to save your changes?");
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		msgBox.exec();
		return;
	}
	QFileInfoList fileList;
	for(auto &index : selections)
		if( 0 == index.column() )
			fileList << proxyModel->fileInfo(index);
	emit setFileAction(fileList, expDir);
}
