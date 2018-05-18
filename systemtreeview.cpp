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
	auto rootPath = parentWindow->getRoot();

	connect(fsModel, SIGNAL(splashText(QString,int,QColor)),
			this, SIGNAL(splashText(QString,int,QColor)));

	auto runner = QtConcurrent::run([&, rootPath](){
		fsModel->scanRoot(rootPath);
	});


	qDebug()<<"Root: "<<rootPath;
	auto rootIndex = model->setRootPath(rootPath);
	model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 1; i < model->columnCount(); ++i)
		hideColumn(i);

	fsModel->setSourceModel(model);
	fsModel->setDynamicSortFilter(false);
	setModel(fsModel);
	setRootIndex(fsModel->mapFromSource(rootIndex));


	for (int i = 1; i < model->columnCount(); ++i)
		hideColumn(i);

	connect(selectionModel(), &QItemSelectionModel::currentChanged,
			[&](QModelIndex current, QModelIndex){
				emit changeDir(fsModel->fileInfo(current).absoluteFilePath());
			});

}

void SystemTreeView::init(QString& startDir){

	qDebug()<<"startDir: "<<startDir;
	runner.waitForFinished();



	auto idx = fsModel->fileIndex(startDir);
	//qDebug()<<"idx: "<<idx.isValid();
	//setCurrentIndex(idx);
	expand(idx);
	scrollTo(idx);

	QDir dir(startDir);

	while(dir.cdUp()){

		runner = fsModel->scanTreeAsync(dir.absolutePath());
		runner.waitForFinished();
		auto idx = fsModel->fileIndex(dir.absolutePath());
		//setCurrentIndex(idx);
		//qDebug()<<"idx: "<<idx.isValid();
		expand(idx);
		//setCurrentIndex(idx);
		scrollTo(idx);

	}


	idx = fsModel->fileIndex(startDir);
	//qDebug()<<"idx: "<<idx.isValid();

	expand(idx);
	scrollTo(idx);
	setCurrentIndex(idx);

}

QString SystemTreeView::getCurrentDir(){
	auto idx = currentIndex();
	qDebug()<<"currentDir";
	return fsModel->fileInfo(idx).absoluteFilePath();
}

void SystemTreeView::initDir(QString& startDir){
	auto idx = fsModel->fileIndex(startDir);
	//qDebug()<<"idx: "<<idx.isValid();
	setCurrentIndex(idx);
	expand(idx);
	scrollTo(idx);

}
