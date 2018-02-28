#ifndef IMGTHUMBNAILDELEGATE_H
#define IMGTHUMBNAILDELEGATE_H
#include <QItemDelegate>
#include <QPainter>
#include <QFileSystemModel>
#include <QImageReader>
#include <QDebug>
#include <QMap>
#include <atomic>
#include <QPixmapCache>
#include "thumbnailsfilemodel.h"
class ImgThumbnailDelegate : public QItemDelegate{
	Q_OBJECT
public:
	explicit ImgThumbnailDelegate(QMap<QString, QPixmap>& cache, QObject *parent);
	void setModel(ThumbnailsFileModel* model){this->model = model;}
	void prepareExit(){isExiting = true;}
	void undoExit(){isExiting = false;}

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
					const QModelIndex &index) const override;

signals:


private:
	ThumbnailsFileModel* model;
	QPixmapCache cache;
	int flags;
	QMap<QString, QPixmap>& currentCache;
	std::atomic_bool isExiting = false;
};

#endif // IMGTHUMBNAILDELEGATE_H
