#include "systemtreeview.h"

SystemTreeView::SystemTreeView(QWidget *parent) : QTreeView(parent){
	fsModel = new ThumbnailsFileModel(this);
	auto model = new QFileSystemModel(this);
	model->setRootPath(QDir::rootPath());
	model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

	fsModel->setSourceModel(model);
	setModel(fsModel);

	for (int i = 1; i < model->columnCount(); ++i)
		hideColumn(i);

	connect(selectionModel(), &QItemSelectionModel::currentChanged,
			[&](QModelIndex current, QModelIndex){
		emit changeDir(fsModel->fileInfo(current).absoluteFilePath());
			});

	connect(this, SIGNAL(expanded(const QModelIndex &)),
			fsModel, SLOT(expanded(const QModelIndex &)));
}

void SystemTreeView::init(QString& startDir){

	//fsModel->init(startDir);
	auto idx = fsModel->fileIndex(startDir);
	expand(idx);
	//auto idx = fsModel->fileIndex(startDir);

	//qDebug()<<"Idx is valid: "<<idx;
	setCurrentIndex(idx);
	scrollTo(idx);
}

QString SystemTreeView::getCurrentDir(){
	auto idx = currentIndex();
	return fsModel->fileInfo(idx).absoluteFilePath();
}
