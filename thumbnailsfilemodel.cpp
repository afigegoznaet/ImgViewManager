#include "thumbnailsfilemodel.h"
#include "imglistview.h"
#include "systemtreeview.h"

ThumbnailsFileModel::ThumbnailsFileModel(QObject *parent)
	: QSortFilterProxyModel(parent){
	filter << "*.png";
	filter << "*.jpeg";
	filter << "*.jpg";

}


bool ThumbnailsFileModel::hasImages(const QModelIndex& dirIndex, bool isSource) const{

	//locker.lock();
	auto info = fileInfo(dirIndex, isSource);
	if(!info.isDir())
		return false;
	QDir dir(info.absoluteFilePath());
	dir.setNameFilters(filter);
	int cnt = dir.count();
	//locker.unlock();
	if(cnt)
		return true;
	return false;
}

QFileInfo ThumbnailsFileModel::fileInfo(const QModelIndex &index, bool isSource) const{
	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	if(isSource)
		return source->fileInfo(index);;
	return source->fileInfo(mapToSource(index));;
}

QModelIndex ThumbnailsFileModel::fileIndex(const QString &path) const{
	auto idx = (dynamic_cast<QFileSystemModel*>(this->sourceModel()))
				->index(path, 0);
	return mapFromSource(idx);
}

bool ThumbnailsFileModel::isVisible(const QModelIndex& parent)const{
	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	if(source->hasChildren(mapToSource(parent)) ){
		//qDebug()<< source->rowCount(mapToSource(parent));
		if(hasImages(parent))
			return true;

		QDir dir(fileInfo(parent).absoluteFilePath());
		if(!(dir.dirName().compare(".") && dir.dirName().compare("..") && dir.dirName().length()))
			return true;
		auto dirEntries = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
		//qDebug()<<dirEntries.count();
		for(auto& entry : dirEntries){
			auto idx = fileIndex(entry.absoluteFilePath());
			//qDebug()<<entry.absoluteFilePath();
			//qDebug()<<"Idx is valid: "<<idx.isValid();
			if(isVisible(idx)){
				qDebug()<<entry.absolutePath();
				return true;
			}

		}
	}
	return false;
}


bool ThumbnailsFileModel::hasPics(const QModelIndex& parent)const{


	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	QDir dir(source->fileInfo(parent).absoluteFilePath());
	//qDebug()<<"Check if visible: "<<dir.absolutePath();
	if(source->hasChildren(parent) ){
		if(hasImages(parent, true)){
			treeMap[dir.absolutePath()] = true;
			return true;
		}



		if(!(dir.dirName().compare(".") && dir.dirName().compare("..") && dir.dirName().length()))
			return true;
		auto dirEntries = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
		//qDebug()<<dirEntries.count();

		for(auto& entry : dirEntries){
			//locker.lock();
			auto idx = source->index(entry.absoluteFilePath());
			//locker.unlock();
			//qDebug()<<entry.absoluteFilePath();
			//qDebug()<<"Idx is valid: "<<idx.isValid();

			if(hasPics( idx )){
				//qDebug()<<entry.absolutePath();
				treeMap[dir.absolutePath()] = true;
				return true;
			}
		}
	}



	treeMap[dir.absolutePath()] = false;
	return false;
}


void ThumbnailsFileModel::expanded(const QModelIndex &index){

	//qDebug()<<"Expanded";
	//qDebug()<<fileInfo(index).absoluteFilePath();
	//qDebug()<<"Visible: "<<hasPics(mapToSource(index));
}

bool ThumbnailsFileModel::filterAcceptsRow(int source_row,
						const QModelIndex &source_parent) const{

	QFileSystemModel *sm = qobject_cast<QFileSystemModel*>(sourceModel());


	auto pIdx = sm->index(source_row, 0, source_parent);

	if(qobject_cast<ImgListView*>(parent())){
		if(sm->rootPath().compare(sm->fileInfo(pIdx).absolutePath()))
			return true;
		if(sm->fileInfo(pIdx).isDir())
			return false;
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	}else{
		QDir dir(sm->fileInfo(pIdx).absoluteFilePath());
		//if(dir.absolutePath().contains(sm->fileInfo(pIdx).absolutePath()))
			//return true;
		bool res1 = treeMap.contains(dir.absolutePath());
		if(res1)
			return treeMap[dir.absolutePath()];

		QString path = dir.absolutePath();
		if(path.contains("/proc"))
			return false;
		//locker.lock();
		//treeParser = QtConcurrent::run([&, path](){
			qDebug()<<"Path: "<<path;
			QFileSystemModel *asd = qobject_cast<QFileSystemModel*>(sourceModel());
			bool res = hasPics(asd->index(path,0));
			qDebug()<<"Trying to update: "<<res;
			return res;

		//});

		//treeParser.waitForFinished();
		//locker.unlock();
		return treeParser.result();

	}


}

QDir ThumbnailsFileModel::rootDirectory() const{
	QFileSystemModel *sm = qobject_cast<QFileSystemModel*>(sourceModel());
	return sm->rootDirectory();
}

void ThumbnailsFileModel::setNameFilters(const QStringList &filters){
	QFileSystemModel *sm = qobject_cast<QFileSystemModel*>(sourceModel());
	sm->setNameFilters(filters);
}

QModelIndex ThumbnailsFileModel::setRootPath(const QString &newPath){
	QFileSystemModel *sm = qobject_cast<QFileSystemModel*>(sourceModel());
	return sm->setRootPath(newPath);

}

void ThumbnailsFileModel::init(QString& startDir){
	qDebug()<<"Init: "<<startDir;
	//QtConcurrent::run([&,startDir](){

		//locker.lock();
		//treeParser.waitForFinished();
		//treeParser = QtConcurrent::run([&,startDir](){
			QFileSystemModel *asd = qobject_cast<QFileSystemModel*>(sourceModel());
			if(hasPics(asd->index(startDir,0))){
				qDebug()<<"Init folder has pics: "<<startDir;
				auto tree = qobject_cast<SystemTreeView*>(parent());
				//tree->setCurrentIndex(asd->index(startDir,0));
				//tree->scrollTo(asd->index(startDir,0));
				//return true;
			}
			//return false;
		//});
		//locker.unlock();
	//});
	qDebug()<<"Init: "<<startDir;
}

