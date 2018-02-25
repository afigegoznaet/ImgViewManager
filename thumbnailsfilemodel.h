#ifndef THUMBNAILSFILEMODEL_H
#define THUMBNAILSFILEMODEL_H
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QDebug>

class ThumbnailsFileModel : public QSortFilterProxyModel{
	Q_OBJECT
public:
	explicit ThumbnailsFileModel(QObject *parent = Q_NULLPTR);
	QFileInfo fileInfo(const QModelIndex &fileIndex) const;
	QModelIndex fileIndex(const QString &path) const;
	QDir rootDirectory() const;
	void setNameFilters(const QStringList &filters);
	QModelIndex setRootPath(const QString &newPath);
signals:
public slots:
	void expanded(const QModelIndex &fileIndex);
private:
	bool isVisible(const QModelIndex&parent) const;
	bool hasImages(const QModelIndex& dirIndex) const;
	bool filterAcceptsRow(int source_row,
						  const QModelIndex &source_parent) const override;
	QStringList filter;
};

#endif // THUMBNAILSFILEMODEL_H
