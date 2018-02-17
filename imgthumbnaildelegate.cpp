#include "imgthumbnaildelegate.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QObject* parent)
	: QItemDelegate(parent){
	QPixmapCache::setCacheLimit(1000*QPixmapCache::cacheLimit());
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	QItemDelegate::paint(painter, option, index);
	if(index.isValid()){

		auto filePath = model->filePath(index);

		QPixmap pm;
		if (QPixmapCache::find(filePath, &pm)) {
			qDebug()<<true;
			painter->drawPixmap(option.rect.left()+1, option.rect.top()+1, pm);
			QItemDelegate::paint(painter, option, index);
			return;
		}

		qDebug()<<false;
		QRect tempRect;
		tempRect.setLeft(option.rect.left()+1);
		tempRect.setTop(option.rect.top()+1);
		tempRect.setRight(option.rect.right()-1);
		tempRect.setBottom(option.rect.bottom()-1);

		QImageReader reader(filePath);
		auto picSize = reader.size();
		double coef = picSize.height()*1.0/picSize.width();
		QSize newSize(tempRect.width(), tempRect.height());

		if(coef>1)
			newSize.setWidth(newSize.width()/coef);
		else
			newSize.setHeight(newSize.height()*coef);
		qDebug()<<index.row();
		reader.setScaledSize(newSize);
		QImage image = reader.read();

		pm = QPixmap::fromImage(image);
		painter->drawPixmap(option.rect.left()+1, option.rect.top()+1, pm);
		QPixmapCache::insert(filePath, pm);
		QItemDelegate::paint(painter, option, index);

	}
}
