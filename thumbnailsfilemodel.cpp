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

	if(!dirIndex.isValid())
		return false;
	auto info = fileInfo(dirIndex, isSource);

	if(!info.isDir())
		return false;
	QDir dir(info.absoluteFilePath());
	dir.setNameFilters(filter);
	uint cnt = dir.count();
	//locker.unlock();
	if(cnt)
		return true;
	return false;
}

QFileInfo ThumbnailsFileModel::fileInfo(const QModelIndex &index, bool isSource) const{
	if(!index.isValid())
		return QFileInfo();
	QPersistentModelIndex idx(index);
	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	if(isSource)
		return source->fileInfo(idx);;

	return source->fileInfo(mapToSource(idx));;
}

QModelIndex ThumbnailsFileModel::fileIndex(const QString &path) const{
	QPersistentModelIndex idx = (dynamic_cast<QFileSystemModel*>(this->sourceModel()))
				->index(path, 0);
	if(!idx.isValid())
		return QModelIndex();
	return mapFromSource(idx);
}



bool ThumbnailsFileModel::hasPics(const QModelIndex& idx)const{

	if(!idx.isValid())
		return false;
	QPersistentModelIndex parent(idx);
	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	//qDebug()<<"hasPics";
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


bool ThumbnailsFileModel::filterAcceptsRow(int source_row,
						const QModelIndex &source) const{

/*
	if(!source.isValid())
		qDebug()<<"Parsing an invalid parent " << source_row <<" "<< source;
	else
		qDebug()<<"Parsing a valid parent " << source_row <<" "<< source;

	if(!source_parent.isValid())
		qDebug()<<"Parsing an invalid parent " << source_row <<" "<< source_parent;
	else
		qDebug()<<"Parsing a valid parent " << source_row <<" "<< source_parent;*/
	QPersistentModelIndex source_parent(source);
	QFileSystemModel *sm = qobject_cast<QFileSystemModel*>(sourceModel());
	QPersistentModelIndex pIdx = sm->index(source_row, 0, source_parent);
	QString path1 = sm->rootPath();
	//qDebug()<<"acceptRow";
	QString path2 = sm->fileInfo(pIdx).absolutePath();
	QDir pt(path2);


	if(qobject_cast<ImgListView*>(parent())){

		//return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

		if( pt.isReadable() && path1.compare(path2))
			return true;
		if(sm->fileInfo(pIdx).isDir())
			return false;
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	}else{


		if(!pt.isReadable() )
			return false;
		//return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
		QDir dir(sm->fileInfo(pIdx).absoluteFilePath());

		bool res1 = treeMap.contains(dir.absolutePath());
		if(res1)
			return treeMap[dir.absolutePath()];

		QString path = dir.absolutePath();
		if(path.startsWith("/proc"))
			return false;

		QFileSystemModel *asd = qobject_cast<QFileSystemModel*>(sourceModel());
		return hasPics(asd->index(path,0));
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
	QFileSystemModel *asd = qobject_cast<QFileSystemModel*>(sourceModel());
	hasPics(asd->index(startDir,0));
}

