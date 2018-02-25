#ifndef IMGTHUMBNAILDELEGATE_H
#define IMGTHUMBNAILDELEGATE_H
#include <QItemDelegate>
#include <QPainter>
#include <QFileSystemModel>
#include <QImageReader>
#include <QDebug>
#include <QPixmapCache>
#include "thumbnailsfilemodel.h"
class ImgThumbnailDelegate : public QItemDelegate{
	Q_OBJECT
public:
	explicit ImgThumbnailDelegate(QObject *parent = Q_NULLPTR);
	void setModel(ThumbnailsFileModel* model){this->model = model;}

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
					const QModelIndex &index) const override;

signals:

public slots:


private:
	ThumbnailsFileModel* model;
	QPixmapCache cache;
	int flags;
};

#endif // IMGTHUMBNAILDELEGATE_H
