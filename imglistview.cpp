#include "imglistview.h"
#include "mainwindow.h"

ImgListView::ImgListView(QWidget *parent) : QListView(parent), stopPrefetching(false){
	fsModel = new QFileSystemModel(this);
	recursiveModel = new QStandardItemModel(this);

	auto parentWindow = qobject_cast<MainWindow*>(parent);
	auto parentObject = parent->parent();
	while(nullptr == parentWindow){
		parentWindow = qobject_cast<MainWindow*>(parentObject);
		parentObject = parentObject->parent();
	}

	qDebug()<<parentWindow->getRoot();
	fsModel->setRootPath(parentWindow->getRoot());
	fsModel->setFilter(QDir::NoDotAndDotDot);

	namedFilters << "*.png";
	namedFilters << "*.jpeg";
	namedFilters << "*.jpg";

	fsModel->setNameFilters(namedFilters);
	fsModel->setNameFilterDisables(false);

	//setModel(fsModel);
	setModel(recursiveModel);

	thumbnailPainter = new ImgThumbnailDelegate(thumbnailsCache, this);
	//thumbnailPainter->setModel(fsModel);
	thumbnailPainter->setModel(recursiveModel);
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
	selectionModel()->setModel(recursiveModel);
	setMovement(Movement::Static);

	copyDialog = new ProgressDialog(this);

	connect(this, SIGNAL(setFileAction(QFileInfoList,QString)),
			copyDialog, SLOT(processFileAction(QFileInfoList,QString)));
	connect(this, SIGNAL(callFullUpdate()), this, SLOT(update()), Qt::QueuedConnection);


	dirLoadBar = new QProgressBar(this);
	dirLoadBar->setMinimum(0);
	dirLoadBar->setMaximum(0);
	dirLoadBar->resize(QSize(300, 25));
	dirLoadBar->setStyleSheet("background:transparent");
	dirLoadBar->setAttribute(Qt::WA_TranslucentBackground);
	dirLoadBar->move(600,600);
	dirLoadBar->setVisible(false);

	connect(this, &ImgListView::progressSetVisible, dirLoadBar, &QProgressBar::setVisible, Qt::QueuedConnection);

	connect(this, &ImgListView::progressSetMaximum, dirLoadBar, &QProgressBar::setMaximum, Qt::QueuedConnection);
	connect(this, &ImgListView::progressSetValue, dirLoadBar, &QProgressBar::setValue, Qt::QueuedConnection);

	openAction = m_menu.addAction("&Open image",[&](){
		QDesktopServices::openUrl(QUrl::fromLocalFile( recursiveModel->data(indexAt(mapFromGlobal(QCursor::pos()))).toString()  ));
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

//	recursiveModel->setHeaderData()
}

void ImgListView::changeDir(QString dir){

	auto region = geometry();
	dirLoadBar->move((QPoint(region.left()+(region.width() - dirLoadBar->width())/2,
					 region.top()+(region.height() - dirLoadBar->height())/2)));

	stopPrefetching = true;

	QDir parent(dir);
	parent.cdUp();
	//qDebug()<<dir<<"_"<<parent.absolutePath();
	//setRootIndex(fsModel->index(parent.absolutePath()));

	fsModel->setRootPath(dir);
	applyFilter(filterText);

	//setRootIndex(fsModel->index(dir));
	//applyFilter(filterText);
	prefetchProc.waitForFinished();
	stopPrefetching = false;
	//applyFilter(filterText);
	thumbnailPainter->stopDrawing();
	thumbnailsCache.clear();
	recursiveModel->clear();
	thumbnailPainter->resumeDrawing();
	prefetchProc = QtConcurrent::run([&](){prefetchThumbnails();});
	//emit callFullUpdate();

}

void ImgListView::prefetchThumbnails(){

	QStringList dirs;
	dirs << fsModel->rootDirectory().absolutePath();
	getDirs(dirs.first(), dirs);

	QStringList fileList;
	for(auto dirEntry : dirs){
		if(stopPrefetching)
			break;
		QDir dir(dirEntry);
		auto dirEntries = dir.entryInfoList(namedFilters);
		for(auto& fileInfo : dirEntries ){
			if(stopPrefetching)
				break;
			if(fileInfo.isDir())
				continue;
			fileList<<fileInfo.absoluteFilePath();
		}
	}

	recursiveModel->insertRows(0, fileList.count());

	int i=0;
	for(auto fileName : fileList){
		if(stopPrefetching)
			break;

		auto item = new QStandardItem();
		item->setData(QIcon(":/Images/spinner.svg"), Qt::DecorationRole);
		item->setData(fileName, Qt::DisplayRole);

		recursiveModel->setItem(i++, item);
		qDebug()<<"F!: "<<fileName;
		emit callFullUpdate();
	}

	for(auto dirEntry : dirs){
		if(stopPrefetching)
			break;
		QDir dir(dirEntry);
		QString fileName = dir.absolutePath();
		fileName +="/.kthumbnails";
		QFile thumbnailsFile(fileName);

		QDataStream in (&thumbnailsFile);
		in.setVersion(QDataStream::Qt_5_7);
		if(thumbnailsFile.open(QIODevice::ReadOnly))
			in>> thumbnailsCache;
		thumbnailsFile.close();
		int countAtStart = thumbnailsCache.count();

		auto dirEntries = dir.entryInfoList(namedFilters);

		emit progressSetVisible(true);
		emit progressSetMaximum(dirEntries.count());
		int counter = 0 ;


		for(auto& fileInfo : dirEntries ){
			if(stopPrefetching)
				break;
			if(fileInfo.isDir())
				continue;
			auto item = recursiveModel->findItems(fileInfo.absoluteFilePath()).first();

			emit progressSetValue(counter++);

			auto currentFileName = fileInfo.absoluteFilePath();
			auto tcEntry = thumbnailsCache.constFind(currentFileName);
			if(tcEntry == thumbnailsCache.constEnd()) {
				QSize iconSize = this->iconSize();
				QImageReader reader(currentFileName);
				auto picSize = reader.size();
				if(picSize.width()>iconSize.width() || picSize.height()>iconSize.height()){
					auto picSize = reader.size();
					double coef = picSize.height()*1.0/picSize.width();
					if(coef>1)
						iconSize.setWidth(iconSize.width()/coef);
					else
						iconSize.setHeight(iconSize.height()*coef);
					reader.setScaledSize(iconSize);
				}

				reader.setAutoTransform(true);
				reader.setQuality(15);

				if(stopPrefetching)
					break;

				auto img = reader.read();
				//thumbnailPainter->stopDrawing();
				item->setIcon(QPixmap::fromImage(img));
				thumbnailsCache.insert(currentFileName,
					img.convertToFormat(QImage::Format_RGB444,Qt::DiffuseDither));
				//thumbnailPainter->resumeDrawing();
			}else
				item->setIcon(QPixmap::fromImage(*tcEntry));


			//qDebug()<<"Item data: "<<item->data(Qt::DisplayRole);
			//qDebug()<<"Item icon: "<<item->data(Qt::DecorationRole);
			//QPersistentModelIndex idx = fsModel->index(fileInfo.absoluteFilePath());
			if(!stopPrefetching)
				emit callUpdate(recursiveModel->indexFromItem(item));
		}
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




	emit progressSetVisible(false);
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
		QDesktopServices::openUrl(QUrl::fromLocalFile(recursiveModel->data(index).toString() ));
	}
}

void ImgListView::prepareExit(){
	stopPrefetching = true;
	thumbnailPainter->stopDrawing();
	prefetchProc.waitForFinished();
}

void ImgListView::applyFilter(QString inFilter){

	QString newFilter="*"+inFilter;
	QStringList newFilters;
	filterText = inFilter;
	for(auto &filter : namedFilters){
		newFilters<<newFilter+filter;
	}

	QDir dir1(fsModel->rootDirectory().absolutePath());
	QDir dir2(fsModel->rootDirectory().absolutePath());
/*
	int cnt1 = dir1.entryInfoList(namedFilters, QDir::Files | QDir::NoDotAndDotDot).count();
	int cnt2 = dir2.entryInfoList(newFilters, QDir::Files | QDir::NoDotAndDotDot).count();

	qDebug()<<cnt1<<" "<<cnt2;
	qDebug()<<dir1.entryInfoList(namedFilters, QDir::Files | QDir::NoDotAndDotDot)<<" "<<cnt1;
	qDebug()<<dir2.entryInfoList(newFilters, QDir::Files | QDir::NoDotAndDotDot)<<" "<<cnt2;
*/

	emit numFiles(
				dir1.entryInfoList(namedFilters, QDir::Files | QDir::NoDotAndDotDot).count(),
				dir2.entryInfoList(newFilters, QDir::Files | QDir::NoDotAndDotDot).count()
				);

	fsModel->nameFilters().clear();
	fsModel->setNameFilters(newFilters);
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
						 fsModel->rootDirectory().absolutePath());
	selector.setFileMode(QFileDialog::DirectoryOnly);
	if (!selector.exec())
		return;
	else
		selector.selectedFiles();

	QString expDir = selector.selectedFiles().first();

	if(0 == expDir.compare(fsModel->rootDirectory().absolutePath())){
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
			fileList << fsModel->fileInfo(index);
	emit setFileAction(fileList, expDir);
}

void ImgListView::mousePressEvent(QMouseEvent *event){
	if(event->button() == Qt::RightButton){
		auto pointedIndex = indexAt(event->pos());
		if(!pointedIndex.isValid())
			openAction->setDisabled(true);

		auto selectionsCount = selectionModel()->selection().indexes().count();
		//qDebug()<<"selections: "<<selectionsCount;
		if(selectionsCount < 1)
			exportAction->setDisabled(true);

		if(pointedIndex.isValid() || selectionsCount > 0){
			QString text = "";
			if(selectionsCount > 1 ){
				fi_selectedFiles->setText("Selected files: \t"+QString::number(selectionsCount));
				fi_fileFormat->setText("");
				fi_bitDepth->setText("");
				fi_grayScale->setText("");
				fi_size->setText("");
				fi_alpha->setText("");
			}else{
				if(!pointedIndex.isValid())
					pointedIndex = selectionModel()->selection().indexes().first();
				auto fileName = fsModel->fileInfo(pointedIndex).absoluteFilePath();
				QImageReader reader(fileName);
				reader.setDecideFormatFromContent(true);

				QImage img = reader.read();

				qint64 size = QFile(fileName).size();
				qDebug()<<"File size: "<<size;
				float gb=0, mb=0, kb=0;
				gb = size / 1000000000.0;
				mb = size / 1000000.0;
				kb = size / 1000.0;
				QString text;
				if(gb>.5){
					text = "File size: \t"+QString::number(gb)+" Gb";
				}else if(mb>.5){
					text = "File size: \t"+QString::number(mb)+" Mb";
				}else if(kb>.5){
					text = "File size: \t"+QString::number(kb)+" Kb";
				}else{
					text = "File size: \t"+QString::number(size)+" b";
				}
				fi_selectedFiles->setText(text);
				fi_fileFormat->setText("File format: \t"
									   + QImageReader::imageFormat(fsModel->fileInfo(pointedIndex).absoluteFilePath()));
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

void ImgListView::getDirs(QString& rootDir, QStringList &dirList){
	QDir dir(rootDir);
	auto currList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

	for(auto dirEntry : currList){

		dirList << dirEntry.absoluteFilePath();
		getDirs(dirEntry.absoluteFilePath(), dirList);
	}

}
