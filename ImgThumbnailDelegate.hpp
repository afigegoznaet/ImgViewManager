#ifndef IMGTHUMBNAILDELEGATE_H
#define IMGTHUMBNAILDELEGATE_H
#include <QItemDelegate>
#include <QPainter>
#include <QImageReader>
#include <QDebug>
#include <QHash>
#include <atomic>
#include <QPixmapCache>
#include <QLabel>
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
	void hidePreview() const{
		previewLabel->setVisible(false);
	}

protected:
	void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
					 const QRect &rect, const QString &text) const override;
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
					const QModelIndex &index) const override;
signals:

public slots:
	void showPreview(bool flag){
		enablePreview = flag;
	}

private:
	void paintPreview(const QModelIndex &index) const;
	void adjustSize() const;
	QLabel* previewLabel;
	mutable QPoint imgPos ;
	mutable QSize imgSize;

	QAbstractItemModel* model;
	int flags;
	std::atomic_bool canDraw;
	std::atomic_bool enablePreview;
	QSize gridSize;
	const QBrush selectionBrush;
	const QBrush hoverBrush;
};

#endif // IMGTHUMBNAILDELEGATE_H
