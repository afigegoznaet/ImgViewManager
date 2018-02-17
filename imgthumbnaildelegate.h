#ifndef IMGTHUMBNAILDELEGATE_H
#define IMGTHUMBNAILDELEGATE_H
#include <QItemDelegate>
#include <QPainter>
#include <QFileSystemModel>
#include <QImageReader>
#include <QDebug>
#include <QtConcurrent>
#include <QPixmapCache>

class ImgThumbnailDelegate : public QItemDelegate{
	Q_OBJECT
public:
	ImgThumbnailDelegate(QObject *parent = Q_NULLPTR);
	void setModel(QFileSystemModel* model){this->model = model;}

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
					const QModelIndex &index) const override;

signals:

public slots:


private:
	QFileSystemModel* model;
	QPixmapCache cache;
};

#endif // IMGTHUMBNAILDELEGATE_H
