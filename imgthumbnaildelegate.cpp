#include "imgthumbnaildelegate.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QObject* parent)
	: QItemDelegate(parent){
	QPixmapCache::setCacheLimit(100*QPixmapCache::cacheLimit());
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	QItemDelegate::paint(painter, option, index);
	if(index.isValid()){

		auto filePath = model->filePath(index);

		QPixmap pm;
		if (QPixmapCache::find(filePath, &pm)) {
			//qDebug()<<true;
			painter->drawPixmap(option.rect.left()+1, option.rect.top()+1, pm);
			QItemDelegate::paint(painter, option, index);
			return;
		}

		//qDebug()<<false;

		//QItemDelegate::paint(painter, option, index);

	}
}
