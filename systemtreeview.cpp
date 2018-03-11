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

	runner = fsModel->scanTreeAsync(parentWindow->getRoot());
	for (int i = 1; i < model->columnCount(); ++i)
		hideColumn(i);

	connect(selectionModel(), &QItemSelectionModel::currentChanged,
			[&](QModelIndex current, QModelIndex){
				emit changeDir(fsModel->fileInfo(current).absoluteFilePath());
			});

}

void SystemTreeView::init(QString& startDir){

	runner.waitForFinished();
	auto idx = fsModel->fileIndex(startDir);
	expand(idx);
	setCurrentIndex(idx);
	qDebug()<<idx.isValid();

	scrollTo(idx);

	runner = fsModel->scanTreeAsync(startDir);
	runner.waitForFinished();

}

QString SystemTreeView::getCurrentDir(){
	auto idx = currentIndex();
	qDebug()<<"currentDir";
	return fsModel->fileInfo(idx).absoluteFilePath();
}
