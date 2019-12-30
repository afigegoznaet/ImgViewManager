#ifndef THUMBNAILSFILEMODEL_H
#define THUMBNAILSFILEMODEL_H
#include <QFileSystemModel>
#include <QtConcurrent>

class SystemTreeView;

class ThumbnailsFileModel : public QSortFilterProxyModel {
	Q_OBJECT
public:
	void prepareExit() { stopPrefetching = true; }
	explicit ThumbnailsFileModel(QObject *parent = Q_NULLPTR);
	QFileInfo	fileInfo(const QModelIndex &fileIndex,
						 bool				isSource = false) const;
	QModelIndex fileIndex(const QString &path) const;
	QDir		rootDirectory() const;
	void		setNameFilters(const QStringList &filters);
	QModelIndex setRootPath(const QString &newPath);
	~ThumbnailsFileModel() override;
	QFuture<bool> scanTreeAsync(const QString &startDir = QDir::rootPath());
	QFuture<void> scanTreeFully(const QString &startDir = QDir::rootPath());
	bool		  hasChildren(const QModelIndex &parent) const override;
	void		  scanRoot(const QString &root);
	QThreadPool & getPool() { return privatePool; }
signals:
	void splashText(const QString &message, int alignment, const QColor &color);
public slots:

private:
	QStringList						  filter;
	mutable QMap<QString, QAtomicInt> treeMap;
	mutable SystemTreeView *		  parentView;
	bool							  stopPrefetching = false;
	mutable QMutex					  scannerMutex;
	QThreadPool						  privatePool;
	QVector<QFuture<void>>			  scanner;
	std::atomic_char				  counter;

	bool hasPics(const QString &scDir) const;
	bool hasImages(const QDir &dir) const;
	bool filterAcceptsRow(int				 source_row,
						  const QModelIndex &source_parent) const override;


	class ScannerRunnable : public QRunnable {
	public:
		ScannerRunnable(ThumbnailsFileModel *host, QString dir);
		void run() override;

	private:
		ThumbnailsFileModel *host;
		const QString		 dir;
	};
};

#endif // THUMBNAILSFILEMODEL_H
