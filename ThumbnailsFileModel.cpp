#include "ThumbnailsFileModel.hpp"
#include "SystemTreeView.hpp"
#include <utility>

ThumbnailsFileModel::ThumbnailsFileModel(QObject *parent)
	: QSortFilterProxyModel(parent), privatePool(this), scanner(8), counter(0) {
	setDynamicSortFilter(true);
	filter << "*.png";
	filter << "*.jpeg";
	filter << "*.jpg";

	parentView = qobject_cast<SystemTreeView *>(parent);
	setDynamicSortFilter(false);
	setRecursiveFilteringEnabled(true);
	privatePool.setMaxThreadCount(4);
	auto model = new QFileSystemModel(this);
	model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
	setSourceModel(model);
	setDynamicSortFilter(false);
	connect(model, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)),
			this, SLOT(rowsToBeInserted(const QModelIndex &, int, int)));
	connect(model, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this,
			SLOT(rowsToBeInserted(const QModelIndex &, int, int)));
}

void ThumbnailsFileModel::rowsToBeInserted(const QModelIndex &parent, int start,
										   int end) {
	qDebug() << "Rows inserted: " << start;
}

ThumbnailsFileModel::~ThumbnailsFileModel() {
	treeMap.clear();
	privatePool.waitForDone(1);
}

bool ThumbnailsFileModel::hasImages(const QDir &dir) const {
	return dir
		.entryInfoList(filter,
					   QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks)
		.count();
}


QFileInfo ThumbnailsFileModel::fileInfo(const QModelIndex &index,
										bool			   isSource) const {
	if (!index.isValid())
		return QFileInfo();
	QPersistentModelIndex idx(index);
	auto source = dynamic_cast<QFileSystemModel *>(sourceModel());
	if (isSource)
		return source->fileInfo(idx);

	return source->fileInfo(mapToSource(idx));
}

QModelIndex ThumbnailsFileModel::fileIndex(const QString &path) const {
	QPersistentModelIndex idx =
		(dynamic_cast<QFileSystemModel *>(this->sourceModel()))->index(path, 0);
	if (!idx.isValid())
		return {};
	return mapFromSource(idx);
}


bool ThumbnailsFileModel::hasPics(const QString &scDir) const {

	if (stopPrefetching)
		return false;
	QDir dir(scDir);

	QtConcurrent::run([this, scDir]() {
		emit parentView->splashText(scDir, Qt::AlignCenter, Qt::white);
	});

	if (hasImages(scDir)) {
		QMutexLocker locker(&scannerMutex);
		if (stopPrefetching)
			return false;
		treeMap.insert(dir.absolutePath(), true);
		return true;
	}

	if (!(scDir.compare(".") && scDir.compare("..") && scDir.length()))
		return true;

	auto dirEntries = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot
										| QDir::NoSymLinks);

	for (auto &entry : dirEntries) {
		if (hasPics(entry.absoluteFilePath())) {
			QMutexLocker locker(&scannerMutex);
			treeMap.insert(scDir, true);
			return true;
		}
	}

	QMutexLocker locker(&scannerMutex);
	if (!stopPrefetching)
		treeMap.insert(scDir, false);
	return false;
}


bool ThumbnailsFileModel::filterAcceptsRow(
	int source_row, const QModelIndex &source_index) const {

	// return QSortFilterProxyModel::filterAcceptsRow(source_row, source_index);
	auto *asd = qobject_cast<QFileSystemModel *>(sourceModel());
	auto  newIndex = asd->index(source_row, 0, source_index);

	auto dirPath = fileInfo(newIndex, true).absoluteFilePath();

	auto res1 = treeMap.constFind(dirPath);
	if (res1 != treeMap.constEnd())
		return *res1;

	if (dirPath.startsWith("/proc"))
		return false;
	if (dirPath.startsWith("C:/Symbols"))
		return false;

	return hasPics(dirPath);
}

QDir ThumbnailsFileModel::rootDirectory() const {
	auto *sm = qobject_cast<QFileSystemModel *>(sourceModel());
	return sm->rootDirectory();
}

void ThumbnailsFileModel::setNameFilters(const QStringList &filters) {
	auto *sm = qobject_cast<QFileSystemModel *>(sourceModel());
	sm->setNameFilters(filters);
}

QModelIndex ThumbnailsFileModel::setRootPath(const QString &newPath) {
	auto *sm = qobject_cast<QFileSystemModel *>(sourceModel());
	return sm->setRootPath(newPath);
}

QFuture<bool> ThumbnailsFileModel::scanTreeAsync(const QString &startDir) {
	return QtConcurrent::run([&, startDir]() {
		auto *		fsModel = qobject_cast<QFileSystemModel *>(sourceModel());
		QModelIndex source_index = fsModel->index(startDir);
		bool		res = false;

		for (int i = 0; i < fsModel->rowCount(source_index); i++) {
			res |= filterAcceptsRow(i, source_index);
			if (res)
				break;
		}

		QDir dir(startDir);
		while (dir.cdUp()) {
			// qDebug()<<"Curr dir: "<<dir.absolutePath();
			QModelIndex source_index = fsModel->index(dir.absolutePath());
			for (int i = 0; i < fsModel->rowCount(source_index); i++)
				filterAcceptsRow(i, source_index);
		}

		return res;
	});
}

ThumbnailsFileModel::ScannerRunnable::ScannerRunnable(ThumbnailsFileModel *host,
													  QString			   dir)
	: host(host), dir(std::move(dir)) {}

void ThumbnailsFileModel::ScannerRunnable::run() {

	if (host->stopPrefetching)
		return;

	QDir currentDir(dir);

	const auto &list =
		currentDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
	for (const auto &dir : list) {
		auto runner = new ScannerRunnable(host, dir.absoluteFilePath());
		host->getPool().start(runner);
		host->scanRoot(dir.absoluteFilePath());
	}
}

QFuture<void> ThumbnailsFileModel::scanTreeFully(const QString &startDir) {

	return QtConcurrent::run([&, startDir]() {
		// this->thread()->setPriority(QThread::LowestPriority);
		QDir currentDir(startDir);

		const auto &list =
			currentDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
		for (const auto &dir : list) {
			auto runner = new ScannerRunnable(this, dir.absoluteFilePath());
			privatePool.start(runner);
		}
	});
}

bool ThumbnailsFileModel::hasChildren(const QModelIndex &parent) const {
	QDir dir = fileInfo(parent).absoluteFilePath();
	return dir
		.entryList(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::NoSymLinks)
		.count();
}

void ThumbnailsFileModel::scanRoot(const QString &root) {
	if (stopPrefetching)
		return;
	QDir dir(root);

	treeMap[root] = hasPics(root);
	for (QFileInfo &item : dir.entryInfoList(
			 QDir::NoDotAndDotDot | QDir::AllDirs | QDir::NoSymLinks)) {
		auto runner = new ScannerRunnable(this, item.absoluteFilePath());
		privatePool.start(runner);
	}
	// connect(treeScanner)
}
