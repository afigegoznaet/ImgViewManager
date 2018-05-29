#include "ThumbnailsFileModel.hpp"
#include "SystemTreeView.hpp"

ThumbnailsFileModel::ThumbnailsFileModel(QObject *parent)
	: QSortFilterProxyModel(parent){
	filter << "*.png";
	filter << "*.jpeg";
	filter << "*.jpg";

	parentView = qobject_cast<SystemTreeView*>(parent);
	setDynamicSortFilter(false);
}



bool ThumbnailsFileModel::hasImages(const QDir &dir) const{
	return dir.entryInfoList(filter, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).count();
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
	qDebug()<<"Idx is valid";
	return mapFromSource(idx);
}


bool ThumbnailsFileModel::hasPics(const QString& scDir)const{

	if(stopPrefetching)
		return false;
	QDir dir(scDir);

	QtConcurrent::run([&,scDir](){
		parentView->splashText(scDir, Qt::AlignCenter, Qt::white);});

	if(hasImages(scDir)){
		treeMap.insert(dir.absolutePath(), true);
		return true;
	}

	if(!(scDir.compare(".") && scDir.compare("..") && scDir.length()))
		return true;
	auto dirEntries = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

	for(auto& entry : dirEntries){
		if(hasPics( entry.absoluteFilePath() )){
			treeMap.insert(scDir, true);
			return true;
		}
	}

	if(!stopPrefetching)
		treeMap.insert(scDir, false);
	return false;
}


bool ThumbnailsFileModel::filterAcceptsRow(int source_row,
						const QModelIndex &source_index) const{

	//return QSortFilterProxyModel::filterAcceptsRow(source_row, source_index);
		QFileSystemModel *asd = qobject_cast<QFileSystemModel*>(sourceModel());
		auto newIndex = asd->index(source_row, 0, source_index);

		auto dirPath = fileInfo(newIndex, true).absoluteFilePath();

		auto res1 = treeMap.constFind(dirPath);
		if(res1 != treeMap.constEnd())
			return *res1;

		if(dirPath.startsWith("/proc"))
			return false;
		if(dirPath.startsWith("C:/Symbols"))
			return false;

		return hasPics(dirPath);


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
		QModelIndex source_index = fsModel->index(startDir);
		bool res = false;

		for(int i=0;i<fsModel->rowCount(source_index);i++){
			res |= filterAcceptsRow(i,	source_index);
			if(res)
				break;
		}

		QDir dir(startDir);
		while(dir.cdUp()){
			qDebug()<<"Curr dir: "<<dir.absolutePath();
			QModelIndex source_index = fsModel->index(dir.absolutePath());
			for(int i=0;i<fsModel->rowCount(source_index);i++)
				filterAcceptsRow(i,	source_index);
		}

		return res;
	});
}

bool ThumbnailsFileModel::hasChildren(const QModelIndex &parent) const {
	QDir dir = fileInfo(parent).absoluteFilePath();
	return dir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::NoSymLinks).count();
}

void ThumbnailsFileModel::scanRoot(QString root){
	if(stopPrefetching)
		return;
	QDir dir(root);

	treeMap[root] = hasPics(root);
	for(QFileInfo &item : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::NoSymLinks))
		scanRoot(item.absoluteFilePath());

}


