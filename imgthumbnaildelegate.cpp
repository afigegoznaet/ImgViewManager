#include "imgthumbnaildelegate.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QObject* parent)
	: QItemDelegate(parent){
	QPixmapCache::setCacheLimit(100*QPixmapCache::cacheLimit());
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	//QItemDelegate::paint(painter, option, index);
	if(index.isValid()){

		auto fileInfo = model->fileInfo(index);

		QPixmap pm;
		if (QPixmapCache::find(fileInfo.absoluteFilePath(), &pm)) {
			//qDebug()<<true;
			painter->drawPixmap(option.rect.left()+1, option.rect.top()+1, pm);
			//QItemDelegate::paint(painter, option, index);
			//return;
		}
	//QPoint point= option.rect.bottomLeft();
	//point.setY(point.ry() - option.fontMetrics.capHeight());
	//painter->drawText(point,fileInfo.fileName());
	painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignBottom, fileInfo.fileName());
	}
}
