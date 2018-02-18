#include "thumbnailsfilemodel.h"

ThumbnailsFileModel::ThumbnailsFileModel(QObject *parent)
	: QFileSystemModel(parent), isInitialized(false){
	filter << "*.png";
	filter << "*.jpeg";
	filter << "*.jpg";
}

bool ThumbnailsFileModel::hasChildren(
		const QModelIndex&parent) const{

	if(!isInitialized)
		return QFileSystemModel::hasChildren(parent);
	qDebug()<<fileInfo(parent).absoluteFilePath();
	qDebug()<<"children";
	if( QFileSystemModel::hasChildren(parent) ){
		qDebug()<<rowCount(parent);
		if(hasImages(parent))
			return true;

		QDir dir(fileInfo(parent).absoluteFilePath());
		auto dirEntries = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
		qDebug()<<dirEntries.count();
		for(auto& entry : dirEntries){
			auto idx = index(entry.absoluteFilePath());
			qDebug()<<entry.absoluteFilePath();
			qDebug()<<"Idx is valid: "<<idx.isValid();
			if(hasChildren(idx))
				return true;
		}
	}
	return false;
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
