#include "ImgListView.hpp"
#include "MainWindow.hpp"
#include <algorithm>

#define SHOWTOTAL

#define MIN_ICON_SIZE 128
#define PREVIEW_SIZE  256
#define ZOOM_THRESHOLD 16
#define MAX_ZOOM (ZOOM_THRESHOLD*5)


ImgListView::ImgListView(QWidget *parent) : QListView(parent), stopPrefetching(false){
	//fsModel = new QFileSystemModel(this);
	recursiveModel0 = new QStandardItemModel(this);
	recursiveModel1 = new QStandardItemModel(this);

	auto parentWindow = qobject_cast<MainWindow*>(parent);
	auto parentObject = parent->parent();
	while(nullptr == parentWindow){
		parentWindow = qobject_cast<MainWindow*>(parentObject);
		parentObject = parentObject->parent();
	}

	qDebug()<<parentWindow->getRoot();

	namedFilters << "*.png";
	namedFilters << "*.jpeg";
	namedFilters << "*.jpg";


	sourceExtensons << "psd";
	sourceExtensons << "eps";
#ifndef _WIN32
	sourceExtensons << "PSD";
	sourceExtensons << "EPS";
#endif

	proxy0 = new ThumbnailsSorter(this);
	proxy1 = new ThumbnailsSorter(this);

	proxy0->setSourceModel(recursiveModel0);
	proxy1->setSourceModel(recursiveModel1);




	thumbnailPainter = new ImgThumbnailDelegate( this);

	setItemDelegate(thumbnailPainter);

#if QT_VERSION_MAJOR == 5 && QT_VERSION_MINOR == 7
	setModel(proxy0);
	thumbnailPainter->setModel(proxy0);

#endif

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
	default_icon_size = std::max<double>(std::ceil((width/8.0)/16.0)*16, MIN_ICON_SIZE);
	setIconSize(QSize(default_icon_size,default_icon_size));
	setGridSize(QSize(iconSize().width()+32, iconSize().height()+32));
	qDebug()<<"Icon size: "<<iconSize();
	qDebug()<<"Grid size: "<<gridSize();
	setLayoutMode (QListView::Batched);



	setUniformItemSizes(true);
	connect(this, SIGNAL(callUpdate(const QString&)),
			this, SLOT(synchronizedUpdate(const QString&)), Qt::QueuedConnection);
	connect(this,SIGNAL(doubleClicked(QModelIndex)),
			this,SLOT(onDoubleClicked()));

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	//selectionModel()->setModel(fsModel);
	//selectionModel()->setModel(proxy);
	setMovement(Movement::Static);

	copyDialog = new ProgressDialog(this);

	connect(this, SIGNAL(setFileAction(QStringList,QString)),
			copyDialog, SLOT(processFileAction(QStringList,QString)));
	connect(this, SIGNAL(callFullUpdate()), this, SLOT(update()), Qt::QueuedConnection);
	connect(this, SIGNAL(filterSignal(QString)), this, SLOT(applyFilter(QString)));

	dirLoadBar = new QProgressBar(this);
	dirLoadBar->setMinimum(0);
	dirLoadBar->setMaximum(0);
	dirLoadBar->resize(QSize(300, 25));
	dirLoadBar->setStyleSheet("background:transparent");
	dirLoadBar->setAttribute(Qt::WA_TranslucentBackground);
	dirLoadBar->move(600,600);
	dirLoadBar->setVisible(false);

#if !defined(QT_DEBUG) || !defined(_WIN32)
	connect(this, &ImgListView::progressSetVisible, [&](bool visible){
		if(!visible)
			return dirLoadBar->setVisible(visible);

		auto region = geometry();
		dirLoadBar->move((QPoint(region.left()+(region.width() - dirLoadBar->width())/2,
						 region.top()+(region.height() - dirLoadBar->height())/2)));
		dirLoadBar->setVisible(visible);
	});
#endif
	connect(this, &ImgListView::progressSetMaximum, dirLoadBar, &QProgressBar::setMaximum, Qt::QueuedConnection);
	connect(this, &ImgListView::progressSetValue, dirLoadBar, &QProgressBar::setValue, Qt::QueuedConnection);

	openAction = m_menu.addAction("&Open image",[&](){
		QDesktopServices::openUrl(QUrl::fromLocalFile( model()->data(indexAt(mapFromGlobal(QCursor::pos()))).toString()  ));
	}, Qt::Key_O);

	openAction->setIconVisibleInMenu(false);
	exportAction = m_menu.addAction("Export &Images",[&](){exportImages();}, Qt::Key_I);
	m_menu.addSeparator();
	//m_menu.addSection("File info");

	fi_selectedFiles = m_menu.addAction("Selected files: 0");
	fi_fileFormat = m_menu.addAction("File format: n\a");
	fi_bitDepth = m_menu.addAction("Color depth: n\a");
	fi_grayScale = m_menu.addAction("Grayscale: n\a");
	fi_size = m_menu.addAction("Image size: n\a");
	fi_alpha = m_menu.addAction("Transparency: n\a");

	//exportAction->setParent(this);
	exportAction->setShortcutContext(Qt::ApplicationShortcut);
	qDebug()<<exportAction->shortcut();
	addAction(exportAction);
	addAction(openAction);

	thumbnailPainter->setGridSize(gridSize());

	connect(this, SIGNAL(resetViewSignal()), this, SLOT(resetViewSlot()));
	connect(this, SIGNAL(sortByPath(bool)), proxy0, SLOT(sortByPath(bool)));
	connect(this, SIGNAL(sortByPath(bool)), proxy1, SLOT(sortByPath(bool)));

//	recursiveModel->setHeaderData()
}

void ImgListView::changeDir(QString dir){

	currentDir = dir;

	stopPrefetching = true;

	prefetchProc.waitForFinished();
	stopPrefetching = false;

	//thumbnailPainter->stopDrawing();

	prefetchProc = QtConcurrent::run([&](){prefetchThumbnails();});
	//

}

void ImgListView::prefetchThumbnails(){

#if QT_VERSION_MAJOR == 5 && QT_VERSION_MINOR > 7

	if(newProxy == proxy0){
		newProxy = proxy1;
		newModel = recursiveModel1;
		oldProxy = proxy0;
		oldModel = recursiveModel0;
	}else{
		newProxy = proxy0;
		newModel = recursiveModel0;
		oldProxy = proxy1;
		oldModel = recursiveModel1;
	}

	newModel->blockSignals(true);

#else

	newProxy = proxy0;
	newModel = recursiveModel0;
	oldProxy = proxy1;
	oldModel = recursiveModel1;

	oldModel->blockSignals(true);
	newModel->blockSignals(true);
	newProxy->setSourceModel(oldModel);

#endif


	newProxy->blockSignals(false);
	oldProxy->blockSignals(true);

	newModel->clear();
	oldModel->clear();
	//oldModel->clear();
	//newProxy->clear();
	emit genericMessage("Scanning "+currentDir);
	QStringList dirs;
	dirs << currentDir;
	getDirs(dirs.first(), dirs);

	for(auto dirEntry : dirs){
		QStringList fileList;
		if(stopPrefetching)
			break;

		emit genericMessage("Scanning "+dirEntry);
		QDir dir(dirEntry);
		auto dirEntries = dir.entryInfoList(namedFilters);
		for(auto& fileInfo : dirEntries ){
			if(stopPrefetching)
				return;
			if(fileInfo.isDir())
				continue;
			fileList<<fileInfo.absoluteFilePath();
		}

		QList<QStandardItem*> items;
		for(auto fileName : fileList){
			if(stopPrefetching)
				return;

			auto item = new QStandardItem();
			item->setData(QIcon(":/Images/spinner.svg"), Qt::DecorationRole);
			item->setData(fileName, Qt::DisplayRole);
			item->setData(fileName, Qt::ToolTipRole);

			//item->setText(fileName.split('/').last());
			items << item;
			newModel->appendRow(item);

		}

	}

	if(stopPrefetching)
		return;

	newProxy->sort();
	//proxy->setSourceModel(recursiveModel);
	emit resetViewSignal();



	emit filterSignal(filterText);

	for(auto dirEntry : dirs){
		if(stopPrefetching)
			break;
		QDir dir(dirEntry);
		QString fileName = dir.absolutePath();
		fileName +="/.kthumbnails";

		QFile thumbnailsFile(fileName);

		QMap<QString, QPixmap> oldCache;
		QMap<QString, QPixmap> newCache;
		QDataStream in (&thumbnailsFile);
		in.setVersion(QDataStream::Qt_5_7);
		if(thumbnailsFile.open(QIODevice::ReadOnly)){
			QMap<QString, QPixmap> cacheMap;
			in>> cacheMap;
			for(auto thumbName : cacheMap.keys())
				oldCache.insert(dir.absolutePath()+"/"+thumbName,cacheMap[thumbName]);

		}

		thumbnailsFile.close();

		auto dirEntries = dir.entryInfoList(namedFilters);


		emit progressSetMaximum(dirEntries.count());
		int counter = 0 ;


		for(auto& fileInfo : dirEntries ){
			if(stopPrefetching)
				break;
			if(fileInfo.isDir())
				continue;

			auto item = newModel->findItems(fileInfo.absoluteFilePath(),
												  Qt::MatchFixedString).first();

			emit progressSetValue(counter++);

			auto currentFileName = fileInfo.absoluteFilePath();
			auto tcEntry = oldCache.constFind(currentFileName);
			if(tcEntry == oldCache.constEnd()) {
				emit progressSetVisible(true);
				QSize iconSize(PREVIEW_SIZE,PREVIEW_SIZE);
				QSize imgSize(iconSize);
				QImageReader reader(currentFileName);
				auto picSize = reader.size();
				if(picSize.width()>iconSize.width() || picSize.height()>iconSize.height()){
					auto picSize = reader.size();
					double coef = picSize.height()*1.0/picSize.width();
					if(coef>1)
						imgSize.setWidth(iconSize.width()/coef);
					else
						imgSize.setHeight(iconSize.height()*coef);
					reader.setScaledSize(imgSize);
				}


				reader.setAutoTransform(true);
				reader.setQuality(15);

				if(stopPrefetching)
					break;

				auto img = reader.read();

				QImage newImg(iconSize,QImage::Format_ARGB32);
				newImg.fill(qRgba(0, 0, 0, 0));
				QPainter painter(&newImg);

				if(true){

					int hDelta(0), vDelta(0);

					if(img.width()<iconSize.width())
						hDelta = (iconSize.width() - img.width())/2;
					if(img.height() < iconSize.height())
						vDelta = (iconSize.height() - img.height())/2;

					painter.setRenderHint(QPainter::Antialiasing, true);
					painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
					painter.drawPixmap(hDelta, vDelta, img.width(), img.height(), QPixmap::fromImage(img));
					painter.save();
					painter.restore();
				}

				if(stopPrefetching)
					break;
				QPixmap newPixmap(QPixmap::fromImage(newImg));
				QIcon icon;
				icon.addPixmap(newPixmap.scaled(this->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
				icon.addPixmap(newPixmap.scaled(this->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation), QIcon::Selected);
				item->setIcon(icon);
				newCache.insert(currentFileName, newPixmap);
				//thumbnailPainter->resumeDrawing();
			}else{
				QIcon icon;
				icon.addPixmap(tcEntry->scaled(this->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
				icon.addPixmap(tcEntry->scaled(this->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation), QIcon::Selected);
				item->setIcon(icon);
				//item->setIcon();
				newCache.insert(currentFileName, *tcEntry);
			}

			item->setText(fileInfo.absoluteFilePath());

			if(stopPrefetching)
				break;

			//QMutex locker;
			//locker.lock();
			emit callUpdate( fileInfo.absoluteFilePath() );
			//synchronizer.wait(&locker);

		}
		emit progressSetVisible(false);
		if(newCache.count() != oldCache.count()){
			//qDebug()<<"Saving cache to file";
			if(thumbnailsFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
				thumbnailsFile.resize(0);
				QDataStream out (&thumbnailsFile);
				out.setVersion(QDataStream::Qt_5_7);
				QMap<QString, QPixmap> cacheMap;
				for(auto thumbName : newCache.keys())
					cacheMap.insert(thumbName.section('/', -1),newCache[thumbName]);
				out<<cacheMap;
				thumbnailsFile.flush();
				thumbnailsFile.close();
			}
		}
	}
	atomicMutex.lock();
	oldModel->clear();
	atomicMutex.unlock();

	if(!stopPrefetching)
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
		//QFileInfo info=fsModel->fileInfo(index);
		QDesktopServices::openUrl(QUrl::fromLocalFile(newProxy->data(index).toString() ));
	}
}

void ImgListView::prepareExit(){
	stopPrefetching = true;
	thumbnailPainter->stopDrawing();
	newProxy->clear();
	newModel->clear();
	prefetchProc.waitForFinished();
}

void ImgListView::applyFilter(QString inFilter){

	filterText = inFilter;
	QString newFilter="*"+inFilter;
	newProxy->setFilterWildcard(newFilter);
	newProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

	int totalCount = newModel->rowCount();
	int visCount = newProxy->rowCount();

	if(visCount > totalCount)
		visCount = totalCount;

	emit numFiles( totalCount, visCount );

}


void ImgListView::exportImages(){
	qDebug()<<"export called";
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
	if(exportDir.length() < 1)
		exportDir = currentDir;
	QFileDialog selector(this,"Select output folder",
						 exportDir);
	selector.setFileMode(QFileDialog::DirectoryOnly);
	if (!selector.exec())
		return;
	else
		selector.selectedFiles();

	exportDir = selector.selectedFiles().first();

	if(0 == exportDir.compare(currentDir)){
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
	QStringList fileList;
	for(auto &index : selections)
		if( 0 == index.column() )
			 fileList << newModel->itemFromIndex(newProxy->mapToSource(index))
						 ->data(Qt::DisplayRole).toString();

	addHiddenFiles(fileList);
	emit setFileAction(fileList, exportDir);
}

void ImgListView::mousePressEvent(QMouseEvent *event){
	if(event->button() == Qt::RightButton){
		auto pointedIndex = indexAt(event->pos());
		if(!pointedIndex.isValid())
			openAction->setDisabled(true);

		QStringList selectedFiels;

		for(auto &index : selectionModel()->selection().indexes())
			selectedFiels << newModel->itemFromIndex(newProxy->mapToSource(index))->data(Qt::DisplayRole).toString();

		auto selectionsCount = selectedFiels.count();
		if(selectionsCount < 1)
			exportAction->setDisabled(true);
		if(pointedIndex.isValid() || selectionsCount > 0){
			QString text = "";
			if(selectionsCount > 1 ){
				fi_selectedFiles->setText("Selected files: \t"+QString::number(selectionsCount));
				fi_fileFormat->setText("Selected files size: \t"+getTotalSize(selectedFiels));

				addHiddenFiles(selectedFiels);

				fi_bitDepth->setText("hidden source files: \t"+QString::number(selectedFiels.count() - selectionsCount));
				fi_grayScale->setText("hidden source files size: \t"+getTotalSize(selectedFiels, selectionsCount));
#ifdef SHOWTOTAL
				fi_size->setText("Total files: \t"+QString::number(selectedFiels.count()));
				fi_alpha->setText("Total files size: \t"+getTotalSize(selectedFiels));
#else
				m_menu.removeAction(fi_size);
				m_menu.removeAction(fi_alpha);

				fi_size->setText("");
				fi_alpha->setText("");
#endif
			}else{
				if(!pointedIndex.isValid())
					pointedIndex = selectionModel()->selection().indexes().first();
				auto fileName = newModel->itemFromIndex(newProxy->mapToSource(pointedIndex))->data(Qt::DisplayRole).toString();
				QImageReader reader(fileName);
				reader.setDecideFormatFromContent(true);

				QImage img = reader.read();

				//m_menu.addAction(fi_bitDepth);
				//m_menu.addAction(fi_grayScale);
#ifndef SHOWTOTAL
				m_menu.addAction(fi_size);
				m_menu.addAction(fi_alpha);
#endif
				QStringList tempList(fileName);
				fi_selectedFiles->setText("File size: \t" + getTotalSize(tempList));
				fi_fileFormat->setText("File format: \t"
									   + QImageReader::imageFormat(fileName));
				fi_bitDepth->setText("Color depth: \t"+QString::number(img.depth())+"bpp");
				fi_grayScale->setText("Grayscale: \t"+QString(img.allGray()?"true":"false"));
				fi_size->setText("Image size: \t"+QString::number(img.width())+"x"+QString::number(img.height()));
				fi_alpha->setText("Transparency: \t"+QString(img.hasAlphaChannel()?"true":"false"));
			}

			m_menu.exec(QCursor::pos());
		}
		openAction->setEnabled(true);
		exportAction->setEnabled(true);
		return;
	}

	QListView::mousePressEvent(event);
}

void ImgListView::getDirs(const QString &rootDir, QStringList &dirList){
	if(stopPrefetching)
		return;
	QDir dir(rootDir);
	auto currList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

	for(auto dirEntry : currList){

		dirList << dirEntry.absoluteFilePath();
		getDirs(dirEntry.absoluteFilePath(), dirList);
	}

}

QString ImgListView::getTotalSize(QStringList& files, int skipFirstNfiles){
	qint64 totalSize = 0;
	for(auto fileName : files)
		if( skipFirstNfiles > 0 && skipFirstNfiles--)
			continue;
		else
			totalSize += QFile(fileName).size();

	float gb=0, mb=0, kb=0;
	gb = totalSize / 1000000000.0;
	mb = totalSize / 1000000.0;
	kb = totalSize / 1000.0;
	QString text;
	if(gb>.5){
		text = QString::number(gb, 'f', 1)+" GB";
	}else if(mb>.5){
		text = QString::number(mb, 'f', 1)+" MB";
	}else if(kb>.5){
		text = QString::number(kb, 'f', 1)+" KB";
	}else{
		text = QString::number(totalSize)+" B";
	}
	return text;
}

void ImgListView::addHiddenFiles(QStringList& fileList){
	for(auto filePath : fileList){
		QFileInfo inf(filePath);

		for( auto newSuffix : sourceExtensons ){
			QString newFile(filePath);
			newFile.replace(
					filePath.lastIndexOf(inf.completeSuffix()), 5, newSuffix);
			if(QFile::exists(newFile))
				fileList<<newFile;
		}
	}

}

void ImgListView::checkSelections(QItemSelection, QItemSelection){

	int selectedCount = selectionModel()->selectedIndexes().count();
	if(selectedCount <1 )
		return applyFilter(filterText);
	QString info = QString::number(selectedCount) + " selected of "+QString::number(newModel->rowCount());
	emit genericMessage(info);
}

void ImgListView::resetViewSlot(){

	newProxy->setSourceModel(newModel);
	thumbnailPainter->resumeDrawing();
#if QT_VERSION_MAJOR == 5 && QT_VERSION_MINOR > 7
	setModel(newProxy);
	thumbnailPainter->setModel(newProxy);
	selectionModel()->setModel(newProxy);
#endif

	connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
			this, SLOT(checkSelections(QItemSelection,QItemSelection)), Qt::UniqueConnection);
}

void ImgListView::synchronizedUpdate(const QString &fileName){

	atomicMutex.lock();


	auto items = newModel->findItems(fileName, Qt::MatchFixedString);
	if(items.count()){
		auto item = items.first();
		auto idx = newModel->indexFromItem(item);

		if(newModel->rowCount() && idx.isValid())
			emit dataChanged(newModel->index(0,0), newModel->index(newModel->rowCount()-1,0), {Qt::DecorationRole});
	}


	atomicMutex.unlock();
}

void ImgListView::setZoom(int zoomDirection){
	if(zoomDirection < 0){
		auto icon_size = iconSize().width();
		icon_size -= ZOOM_THRESHOLD;
		if(icon_size < default_icon_size - MAX_ZOOM)
			return;
		setIconSize(QSize(icon_size,icon_size));
		setGridSize(QSize(iconSize().width()+32, iconSize().height()+32));
	}else if(zoomDirection > 0){
		auto icon_size = iconSize().width();
		icon_size += ZOOM_THRESHOLD;
		if(icon_size > default_icon_size + MAX_ZOOM)
			return;
		setIconSize(QSize(icon_size,icon_size));
		setGridSize(QSize(iconSize().width()+32, iconSize().height()+32));
	}else{
		setIconSize(QSize(default_icon_size,default_icon_size));
		setGridSize(QSize(iconSize().width()+32, iconSize().height()+32));
	}
	emit callFullUpdate();
}
