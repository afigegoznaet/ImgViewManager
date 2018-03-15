#include "thumbnailsfilemodel.h"
#include "systemtreeview.h"

ThumbnailsFileModel::ThumbnailsFileModel(QObject *parent)
	: QSortFilterProxyModel(parent){
	filter << "*.png";
	filter << "*.jpeg";
	filter << "*.jpg";

	parentView = qobject_cast<SystemTreeView*>(parent);

}


bool ThumbnailsFileModel::hasImages(const QModelIndex& dirIndex, bool isSource) const{

	if(!dirIndex.isValid())
		return false;
	auto info = fileInfo(dirIndex, isSource);

	//qDebug()<<info.absolutePath();
	if(!info.isDir())
		return false;
	QDir dir(info.absoluteFilePath());
	//dir.setNameFilters(filter);
	uint cnt = dir.entryInfoList(filter, QDir::Files | QDir::NoDotAndDotDot).count();
	//qDebug()<<cnt;
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


	QString splashMessage = "Scanning:\n"+dir.absolutePath();

	QtConcurrent::run([&,splashMessage](){
		//qDebug()<<splashMessage;
		parentView->splashText(splashMessage, Qt::AlignCenter, Qt::white);});


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
						const QModelIndex &source_index) const{

	//return QSortFilterProxyModel::filterAcceptsRow(source_row, source_index);
		QFileSystemModel *asd = qobject_cast<QFileSystemModel*>(sourceModel());
		auto newIndex = asd->index(source_row, 0, source_index);

		//qDebug()<<newIndex;
		QDir dir(fileInfo(newIndex, true).absoluteFilePath());


		auto res1 = treeMap.constFind(dir.absolutePath());
		if(res1 != treeMap.constEnd())
			return *res1;

		QString path = dir.absolutePath();
		if(path.startsWith("/proc"))
			return false;
		if(path.startsWith("C:/Symbols"))
			return false;


		return hasPics(asd->index(path,0));


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

QFuture<bool> ThumbnailsFileModel::scanTreeAsync(const QString& startDir){
	return QtConcurrent::run([&, startDir](){
		QFileSystemModel *fsModel = qobject_cast<QFileSystemModel*>(sourceModel());
		QPersistentModelIndex source_index = fsModel->index(startDir);
		bool res = false;

		for(int i=0;i<fsModel->rowCount(source_index);i++)
			res |= filterAcceptsRow(i,	source_index);

		QDir dir(startDir);
		while(dir.cdUp()){
			QPersistentModelIndex source_index = fsModel->index(dir.absolutePath());

			for(int i=0;i<fsModel->rowCount(source_index);i++)
				filterAcceptsRow(i,	source_index);

		}
		return res;
	});
}

bool ThumbnailsFileModel::hasChildren(const QModelIndex &parent) const {

	QDir dir = fileInfo(parent).absolutePath();
	for(auto fi : dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs))
		qDebug()<<fi;

	return dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs).count();
}
