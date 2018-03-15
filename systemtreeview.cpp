#include "systemtreeview.h"
#include "mainwindow.h"

SystemTreeView::SystemTreeView(QWidget *parent) : QTreeView(parent){



	fsModel = new ThumbnailsFileModel(this);
	auto model = new QFileSystemModel(this);

	auto parentWindow = qobject_cast<MainWindow*>(parent);
	auto parentObject = parent->parent();
	while(nullptr == parentWindow){
		parentWindow = qobject_cast<MainWindow*>(parentObject);
		parentObject = parentObject->parent();
	}

	qDebug()<<parentWindow->getRoot();
	model->setRootPath(parentWindow->getRoot());
	model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

	connect(fsModel, SIGNAL(splashText(QString,int,QColor)),
			this, SIGNAL(splashText(QString,int,QColor)));

	fsModel->setSourceModel(model);
	setModel(fsModel);
	setRootIndex(fsModel->fileIndex(parentWindow->getRoot()));

	//setModel(model);
	//setRootIndex(model->index(parentWindow->getRoot()));

	runner = fsModel->scanTreeAsync(parentWindow->getRoot());
	for (int i = 1; i < model->columnCount(); ++i)
		hideColumn(i);

	connect(selectionModel(), &QItemSelectionModel::currentChanged,
			[&](QModelIndex current, QModelIndex){
				emit changeDir(fsModel->fileInfo(current).absoluteFilePath());
			});

	runner.waitForFinished();
	QSettings settings;
	auto startDir = settings.value("StartDir",QDir::rootPath()).toString();
	if(startDir.length()<1)
		startDir = QDir::rootPath();
	init(startDir);
	runner.waitForFinished();

	setStyleSheet("\
				  QTreeView::branch:!has-children  {\
						  image: none;\
				  } ");

	connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(expanSionSlot(QModelIndex)));
}

void SystemTreeView::init(QString& startDir){

	runner.waitForFinished();


	QDir dir(startDir);
	dir.cdUp();
	auto idx = fsModel->fileIndex(dir.absolutePath());
	setCurrentIndex(idx);
	expand(idx);
	scrollTo(idx);

	runner = fsModel->scanTreeAsync(startDir);
	runner.waitForFinished();

	idx = fsModel->fileIndex(startDir);
	setCurrentIndex(idx);
	expand(idx);
	scrollTo(idx);


}

QString SystemTreeView::getCurrentDir(){
	auto idx = currentIndex();
	qDebug()<<"currentDir";
	return fsModel->fileInfo(idx).absoluteFilePath();
}

void SystemTreeView::expanSionSlot(const QModelIndex &index){
	int rows = fsModel->rowCount(index);
	qDebug()<<"Parent rows: "<<rows;
	for(int i = 0;i<rows;i++){
		auto childIndex = fsModel->index(i, 0, index);
		if(!fsModel->hasChildren(childIndex)){
			expand(childIndex);
		}else
		for(int j = 0; j < fsModel->rowCount(childIndex); j++){
			auto recChild = fsModel->index(j, 0, childIndex);
			if(!fsModel->hasChildren(recChild)){
				expand(recChild);
			}
		}

	}
}

