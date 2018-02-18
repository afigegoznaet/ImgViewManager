#include "systemtreeview.h"

SystemTreeView::SystemTreeView(QWidget *parent) : QTreeView(parent){
	fsModel = new QFileSystemModel(this);
	auto rootIdx = fsModel->setRootPath(QDir::rootPath());
	fsModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

	setModel(fsModel);
	//setRootIndex(rootIdx);
	//setCurrentIndex(rootIdx);
	for (int i = 1; i < fsModel->columnCount(); ++i)
		hideColumn(i);

	connect(selectionModel(), &QItemSelectionModel::currentChanged,
			[&](QModelIndex current, QModelIndex){
		emit changeDir(fsModel->fileInfo(current).absoluteFilePath());
			});
}

void SystemTreeView::init(QString& startDir){

	auto idx = fsModel->index(startDir);

	qDebug()<<"Idx is valid: "<<idx;
	setCurrentIndex(idx);
	scrollTo(idx);
}

QString SystemTreeView::getCurrentDir(){
	auto idx = currentIndex();
	return fsModel->fileInfo(idx).absoluteFilePath();
}
