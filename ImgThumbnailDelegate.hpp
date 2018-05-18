#ifndef IMGTHUMBNAILDELEGATE_H
#define IMGTHUMBNAILDELEGATE_H
#include <QItemDelegate>
#include <QPainter>
#include <QImageReader>
#include <QDebug>
#include <QHash>
#include <atomic>
#include <QPixmapCache>
#include <QStandardItemModel>
#include <QAbstractItemModel>

class ImgThumbnailDelegate : public QItemDelegate{
	Q_OBJECT
public:

	explicit ImgThumbnailDelegate(QObject *parent);
	void setModel(QAbstractItemModel* model){this->model = model;}
	void stopDrawing(){canDraw = false;}
	void resumeDrawing(){canDraw = true;}
	void setGridSize(QSize gridSize){this->gridSize = gridSize;}
	QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override{
		return gridSize;
	}

protected:
    void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                     const QRect &rect, const QString &text) const override;
signals:


private:
	QAbstractItemModel* model;
	int flags;
	std::atomic_bool canDraw;
	QSize gridSize;

};

#endif // IMGTHUMBNAILDELEGATE_H
