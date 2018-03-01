#ifndef IMGTHUMBNAILDELEGATE_H
#define IMGTHUMBNAILDELEGATE_H
#include <QItemDelegate>
#include <QPainter>
#include <QFileSystemModel>
#include <QImageReader>
#include <QDebug>
#include <QHash>
#include <atomic>
#include <QPixmapCache>
#include "thumbnailsfilemodel.h"
class ImgThumbnailDelegate : public QItemDelegate{
	Q_OBJECT
public:
	explicit ImgThumbnailDelegate(QHash<QString, QPixmap>& cache, QObject *parent);
	void setModel(ThumbnailsFileModel* model){this->model = model;}
	void stopDrawing(){canDraw = true;}
	void resumeDrawing(){canDraw = false;}

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
					const QModelIndex &index) const override;

signals:


private:
	ThumbnailsFileModel* model;
	QPixmapCache cache;
	int flags;
	QHash<QString, QPixmap>& currentCache;
	std::atomic_bool canDraw;
};

#endif // IMGTHUMBNAILDELEGATE_H
