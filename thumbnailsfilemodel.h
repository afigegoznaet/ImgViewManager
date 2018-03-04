#ifndef THUMBNAILSFILEMODEL_H
#define THUMBNAILSFILEMODEL_H
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QMutex>
#include <QFuture>

class ThumbnailsFileModel : public QSortFilterProxyModel{
	Q_OBJECT
public:
	explicit ThumbnailsFileModel(QObject *parent = Q_NULLPTR);
	QFileInfo fileInfo(const QModelIndex &fileIndex, bool isSource = false) const;
	QModelIndex fileIndex(const QString &path) const;
	QDir rootDirectory() const;
	void setNameFilters(const QStringList &filters);
	QModelIndex setRootPath(const QString &newPath);
	~ThumbnailsFileModel(){treeMap.clear();}

signals:

public slots:

private:

	bool hasPics(const QModelIndex&parent) const;
	bool hasImages(const QModelIndex& dirIndex, bool isSource = false) const;
	bool filterAcceptsRow(int source_row,
						  const QModelIndex &source_parent) const override;

	QStringList filter;
	mutable QMap<QString,QAtomicInt> treeMap;
};

#endif // THUMBNAILSFILEMODEL_H
