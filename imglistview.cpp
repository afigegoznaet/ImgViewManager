#include "imglistview.h"
#include "mainwindow.h"

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


	proxy0 = new QSortFilterProxyModel(this);
	proxy1 = new QSortFilterProxyModel(this);

	proxy0->setSourceModel(recursiveModel0);
	proxy1->setSourceModel(recursiveModel1);


	thumbnailPainter = new ImgThumbnailDelegate( this);

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

	connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(checkSelections(QItemSelection,QItemSelection)));
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


//	recursiveModel->setHeaderData()
}

void ImgListView::changeDir(QString dir){

	currentDir = dir;


	stopPrefetching = true;

	prefetchProc.waitForFinished();
	stopPrefetching = false;
	//applyFilter(filterText);
	thumbnailPainter->stopDrawing();
	thumbnailsCache.clear();
	//recursiveModel->clear();
	//proxy->clear();
	thumbnailPainter->resumeDrawing();
	prefetchProc = QtConcurrent::run([&](){prefetchThumbnails();});
	//

}

void ImgListView::prefetchThumbnails(){


	if(recursiveModel0->rowCount()){
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


	newProxy->clear();
	newModel->clear();

	emit genericMessage(currentDir);
	QStringList dirs;
	dirs << currentDir;
	getDirs(dirs.first(), dirs);

	QStandardItem *prototype = new QStandardItem();
	prototype->setData(QIcon(":/Images/spinner.svg"), Qt::DecorationRole);
	prototype->setData("prototype", Qt::DisplayRole);
	prototype->setData("prototype", Qt::ToolTipRole);



	for(auto dirEntry : dirs){
		QStringList fileList;
		if(stopPrefetching)
			break;

		emit genericMessage(dirEntry);
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
			//qDebug()<<"F!: "<<fileName;
			//emit callFullUpdate();

		}

	}

	if(stopPrefetching)
		return;

	//proxy->setSourceModel(recursiveModel);

	newProxy->setSourceModel(newModel);

	setModel(newProxy);
	thumbnailPainter->setModel(newProxy);
	selectionModel()->setModel(newProxy);
	connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
			this, SLOT(checkSelections(QItemSelection,QItemSelection)), Qt::UniqueConnection);


	setLayoutMode (QListView::Batched);

	emit filterSignal(filterText);

	for(auto dirEntry : dirs){
		if(stopPrefetching)
			break;
		QDir dir(dirEntry);
		QString fileName = dir.absolutePath();
#ifdef _WIN32
		fileName +="/.kthumbnailsWIN";
#else
		fileName +="/.kthumbnails";
#endif


		QFile thumbnailsFile(fileName);

		QDataStream in (&thumbnailsFile);
		in.setVersion(QDataStream::Qt_5_7);
		if(thumbnailsFile.open(QIODevice::ReadOnly))
			in>> thumbnailsCache;
		thumbnailsFile.close();
		int countAtStart = thumbnailsCache.count();

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
			auto tcEntry = thumbnailsCache.constFind(currentFileName);
			if(tcEntry == thumbnailsCache.constEnd()) {
				emit progressSetVisible(true);
				QSize iconSize(135,135);
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

					painter.drawPixmap(hDelta, vDelta, img.width(), img.height(), QPixmap::fromImage(img));
					painter.save();
					painter.restore();
				}

				if(stopPrefetching)
					break;
				QPixmap newPixmap(QPixmap::fromImage(newImg));
				item->setIcon(newPixmap.scaled(this->iconSize()));
				thumbnailsCache.insert(currentFileName, newPixmap);
				//thumbnailPainter->resumeDrawing();
			}else
				item->setIcon(tcEntry->scaled(this->iconSize()));
			item->setText(fileInfo.absoluteFilePath());

			if(stopPrefetching)
				break;
			auto idx = newModel->indexFromItem(item);
			if(idx.isValid())
				emit callUpdate(newProxy->mapFromSource( idx));

		}
		emit progressSetVisible(false);
		if(countAtStart != thumbnailsCache.count()){
			//qDebug()<<"Saving cache to file";
			if(thumbnailsFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
				thumbnailsFile.resize(0);
				QDataStream out (&thumbnailsFile);
				out.setVersion(QDataStream::Qt_5_7);
				out<<thumbnailsCache;
				thumbnailsFile.flush();
				thumbnailsFile.close();
			}
		}
	}
	oldModel->clear();
	oldProxy->clear();

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
	QFileDialog selector(this,"Select output folder",
						 currentDir);
	selector.setFileMode(QFileDialog::DirectoryOnly);
	if (!selector.exec())
		return;
	else
		selector.selectedFiles();

	QString expDir = selector.selectedFiles().first();

	if(0 == expDir.compare(currentDir)){
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
			fileList << newModel->itemFromIndex(newProxy->mapToSource(index))->data(Qt::DisplayRole).toString();
	emit setFileAction(fileList, expDir);
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
				fi_bitDepth->setText("");
				fi_grayScale->setText("");
				fi_size->setText("");
				fi_alpha->setText("");
			}else{
				if(!pointedIndex.isValid())
					pointedIndex = selectionModel()->selection().indexes().first();
				auto fileName = newModel->itemFromIndex(newProxy->mapToSource(pointedIndex))->data(Qt::DisplayRole).toString();
				QImageReader reader(fileName);
				reader.setDecideFormatFromContent(true);

				QImage img = reader.read();


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

QString ImgListView::getTotalSize(QStringList& files){
	qint64 totalSize = 0;
	for(auto fileName : files)
		totalSize += QFile(fileName).size();

	float gb=0, mb=0, kb=0;
	gb = totalSize / 1000000000.0;
	mb = totalSize / 1000000.0;
	kb = totalSize / 1000.0;
	QString text;
	if(gb>.5){
		text = QString::number(gb)+" Gb";
	}else if(mb>.5){
		text = QString::number(mb)+" Mb";
	}else if(kb>.5){
		text = QString::number(kb)+" Kb";
	}else{
		text = QString::number(totalSize)+" b";
	}
	return text;
}

void ImgListView::checkSelections(QItemSelection, QItemSelection){

	int selectedCount = selectionModel()->selectedIndexes().count();
	if(selectedCount <1 )
		return applyFilter(filterText);
	QString info = QString::number(selectedCount) + " selected of "+QString::number(newModel->rowCount());
	emit genericMessage(info);
}
