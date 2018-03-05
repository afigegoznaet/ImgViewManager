#include "systemtreeview.h"

SystemTreeView::SystemTreeView(QWidget *parent) : QTreeView(parent){
	fsModel = new ThumbnailsFileModel(this);
	auto model = new QFileSystemModel(this);
	model->setRootPath(QDir::rootPath());
	model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

	connect(fsModel, SIGNAL(splashText(QString,int,QColor)),
			this, SIGNAL(splashText(QString,int,QColor)));

	fsModel->setSourceModel(model);
	setModel(fsModel);

	runner = fsModel->scanTreeAsync();
	for (int i = 1; i < model->columnCount(); ++i)
		hideColumn(i);

	connect(selectionModel(), &QItemSelectionModel::currentChanged,
			[&](QModelIndex current, QModelIndex){
				emit changeDir(fsModel->fileInfo(current).absoluteFilePath());
			});

}

void SystemTreeView::init(QString& startDir){

	//fsModel->init(startDir);
	/*QDir dir(startDir);
	while(dir.cdUp()){
		auto idx = fsModel->fileIndex(dir.absolutePath());
		expand(idx);
	}*/

	runner.waitForFinished();
	auto idx = fsModel->fileIndex(startDir);
	expand(idx);
	setCurrentIndex(idx);
	scrollTo(idx);


	//runner.waitForFinished();
	runner = fsModel->scanTreeAsync(startDir);
	runner.waitForFinished();
	//qDebug()<<runner.resultCount();
}

QString SystemTreeView::getCurrentDir(){
	auto idx = currentIndex();
	qDebug()<<"currentDir";
	return fsModel->fileInfo(idx).absoluteFilePath();
}
