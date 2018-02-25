#ifndef THUMBNAILSFILEMODEL_H
#define THUMBNAILSFILEMODEL_H
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QDebug>

class ThumbnailsFileModel : public QSortFilterProxyModel{
	Q_OBJECT
public:
	explicit ThumbnailsFileModel(QObject *parent = Q_NULLPTR);
	QFileInfo fileInfo(const QModelIndex &index) const;
	QModelIndex index(const QString &path, int column = 0) const;
	QDir rootDirectory() const;
	void setNameFilters(const QStringList &filters);
	QModelIndex setRootPath(const QString &newPath);
signals:
public slots:
	void expanded(const QModelIndex &index);
private:
	bool isVisible(const QModelIndex&parent) const;
	bool hasImages(const QModelIndex& dirIndex) const;
	bool filterAcceptsRow(int source_row,
						  const QModelIndex &source_parent) const;
	QStringList filter;
};

#endif // THUMBNAILSFILEMODEL_H
