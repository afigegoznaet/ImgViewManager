#include "thumbnailsfilemodel.h"

ThumbnailsFileModel::ThumbnailsFileModel(QObject *parent)
	: QSortFilterProxyModel(parent){
	filter << "*.png";
	filter << "*.jpeg";
	filter << "*.jpg";
}


bool ThumbnailsFileModel::hasImages(const QModelIndex& dirIndex) const{

	auto info = fileInfo(dirIndex);
	if(!info.isDir())
		return false;
	QDir dir(info.absoluteFilePath());
	dir.setNameFilters(filter);
	if(dir.count())
		return true;
	return false;
}

QFileInfo ThumbnailsFileModel::fileInfo(const QModelIndex &index) const{
	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	return source->fileInfo(mapToSource(index));;
}

QModelIndex ThumbnailsFileModel::index(const QString &path) const{
	auto idx = (dynamic_cast<QFileSystemModel*>(this->sourceModel()))
				->index(path, 0);
	return mapFromSource(idx);
}

bool ThumbnailsFileModel::isVisible(const QModelIndex& parent)const{
	qDebug()<<"Parent******************************"<<parent;
	qDebug()<<sourceModel()->parent(parent);

	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	qDebug()<<fileInfo(parent).absoluteFilePath();
	qDebug()<<"children";
	if(source->hasChildren(mapToSource(parent)) ){
		qDebug()<< source->rowCount(mapToSource(parent));
		if(hasImages(parent))
			return true;

		QDir dir(fileInfo(parent).absoluteFilePath());
		if(!(dir.dirName().compare(".") && dir.dirName().compare("..") && dir.dirName().length()))
			return true;
		auto dirEntries = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
		qDebug()<<dirEntries.count();
		for(auto& entry : dirEntries){
			auto idx = index(entry.absoluteFilePath());
			qDebug()<<entry.absoluteFilePath();
			qDebug()<<"Idx is valid: "<<idx.isValid();
			if(isVisible(idx))
				return true;
		}
	}
	return false;
}

void ThumbnailsFileModel::expanded(const QModelIndex &index){
	qDebug()<<"Expanded";
	qDebug()<<"Visible: "<<isVisible(index);
}

bool ThumbnailsFileModel::filterAcceptsRow(int source_row,
						const QModelIndex &source_parent) const{

	QFileSystemModel *sm = qobject_cast<QFileSystemModel*>(sourceModel());


	auto pIdx = sourceModel()->index(source_row, 0, source_parent);
/*
	qDebug()<<pIdx.isValid();
	qDebug()<<pIdx;
	qDebug()<< sm->fileInfo(pIdx).absoluteFilePath();
	qDebug()<< sm->rootPath();
	qDebug()<< sm->fileInfo(pIdx).absolutePath();*/
	if(sm->rootPath().compare(sm->fileInfo(pIdx).absolutePath()))
		return true;
	//qDebug()<<QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
	//if (source_parent == sm->index(sm->rootPath()) || source_row <2)
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

	qDebug()<<"!!!: "<<source_row;
	qDebug()<<"Parent: "<<QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

	auto source = dynamic_cast<QFileSystemModel*>(sourceModel());
	qDebug()<< source->fileInfo(pIdx).absoluteFilePath();
	auto idx = mapFromSource(pIdx);
	qDebug()<<idx.isValid();
	if(!idx.isValid())
		return false;
	qDebug()<<idx.isValid();
	qDebug()<<"Filter:"<<fileInfo(idx).baseName();
	qDebug()<<idx;
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
