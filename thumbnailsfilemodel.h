#ifndef THUMBNAILSFILEMODEL_H
#define THUMBNAILSFILEMODEL_H
#include <QFileSystemModel>
#include <QDebug>

class ThumbnailsFileModel : public QFileSystemModel
{
public:
	explicit ThumbnailsFileModel(QObject *parent = Q_NULLPTR);

	bool hasChildren(const QModelIndex&
					 parent = QModelIndex()) const override;
	QStringList filter;
private:
	bool hasImages(const QModelIndex& dirIndex) const;
	bool isInitialized;
};

#endif // THUMBNAILSFILEMODEL_H
