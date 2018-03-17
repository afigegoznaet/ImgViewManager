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

class ImgThumbnailDelegate : public QItemDelegate{
	Q_OBJECT
public:
	explicit ImgThumbnailDelegate(QHash<QString, QImage>& cache, QObject *parent);
	void setModel(QFileSystemModel* model){this->model = model;}
	void stopDrawing(){canDraw = false;}
	void resumeDrawing(){canDraw = true;}

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
					const QModelIndex &index) const override;

signals:


private:
	QFileSystemModel* model;
	QPixmapCache cache;
	int flags;
	QHash<QString, QImage>& currentCache;
	std::atomic_bool canDraw;
};

#endif // IMGTHUMBNAILDELEGATE_H
