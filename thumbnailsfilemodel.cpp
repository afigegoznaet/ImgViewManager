#include "thumbnailsfilemodel.h"
#include "imglistview.h"

ThumbnailsFileModel::ThumbnailsFileModel(QObject *parent)
	: QSortFilterProxyModel(parent){
	filter << "*.png";
	filter << "*.jpeg";
	filter << "*.jpg";
}


bool ThumbnailsFileModel::hasImages(const QModelIndex& dirIndex, bool isSource) const{

	auto info = fileInfo(dirIndex, isSource);
	if(!info.isDir())
		return false;
	QDir dir(info.absoluteFilePath());
	dir.setNameFilters(filter);
	if(dir.count())
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
	//qDebug()<<"Parent******************************"<<parent;
	//qDebug()<<sourceModel()->parent(parent);

	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	//qDebug()<<fileInfo(parent).absoluteFilePath();
	//qDebug()<<"children";
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
			if(isVisible(idx))
				return true;
		}
	}
	return false;
}


bool ThumbnailsFileModel::hasPics(const QModelIndex& parent)const{
	//qDebug()<<"Parent******************************"<<parent;
	//qDebug()<<sourceModel()->parent(parent);

	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	//qDebug()<<fileInfo(parent).absoluteFilePath();
	//qDebug()<<source->fileInfo(parent).absoluteFilePath();
	if(source->hasChildren(parent) ){

		if(hasImages(parent, true))
			return true;

		QDir dir(source->fileInfo(parent).absoluteFilePath());
		if(!(dir.dirName().compare(".") && dir.dirName().compare("..") && dir.dirName().length()))
			return true;
		auto dirEntries = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
		//qDebug()<<dirEntries.count();

		for(auto& entry : dirEntries){

			auto idx = fileIndex(entry.absoluteFilePath());
			//qDebug()<<entry.absoluteFilePath();
			//qDebug()<<"Idx is valid: "<<idx.isValid();
			if(hasPics( mapToSource(idx)))
				return true;
		}
	}
	return false;
}


void ThumbnailsFileModel::expanded(const QModelIndex &index){

	qDebug()<<"Expanded";
	qDebug()<<fileInfo(index).absoluteFilePath();
	qDebug()<<"Visible: "<<isVisible(index);
}

bool ThumbnailsFileModel::filterAcceptsRow(int source_row,
						const QModelIndex &source_parent) const{

	QFileSystemModel *sm = qobject_cast<QFileSystemModel*>(sourceModel());


	auto pIdx = sm->index(source_row, 0, source_parent);

	if(sm->rootPath().compare(sm->fileInfo(pIdx).absolutePath()))
		return true;

	if(sm->fileInfo(pIdx).isDir() && qobject_cast<ImgListView*>(parent()))
		return false;
	else if(sm->fileInfo(pIdx).isDir()){
		if(QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
			try{
				return true;//hasPics(pIdx);
		}catch(std::exception e){
			qDebug()<<e.what();
			return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
		}


	}else
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

	//auto source = dynamic_cast<QFileSystemModel*>(sourceModel());

	auto idx = mapFromSource(pIdx);

	if(!idx.isValid())
		return false;

	return isVisible(idx);
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
