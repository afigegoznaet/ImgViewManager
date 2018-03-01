#include "imgthumbnaildelegate.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QHash<QString, QPixmap> &cache, QObject* parent)
	: QItemDelegate(parent), currentCache(cache), canDraw(true){
	QPixmapCache::setCacheLimit(100*QPixmapCache::cacheLimit());
	flags = Qt::AlignHCenter | Qt::AlignBottom;
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	if(canDraw){
		qDebug()<<"Paint";
		auto fileInfo = model->fileInfo(index);
		qDebug()<<"Got info";
		auto fileName = fileInfo.fileName();
		qDebug()<<"Got name";
		QFontMetrics fm(option.font);
		auto boundingRect = fm.boundingRect(option.rect,flags,fileName);
		boundingRect.setLeft(option.rect.left());
		boundingRect.setRight(option.rect.right());

		auto pixIt = currentCache.constFind(fileName);
		if(canDraw && pixIt != currentCache.constEnd()){
			QPixmap pm = *pixIt;
			painter->drawPixmap(option.rect.left(), option.rect.top(), pm);
		}



		if(canDraw && option.state & QStyle::State_Selected){
			auto brush = option.palette.highlight();
			QRect frame(option.rect);
			painter->setPen(brush.color());
			frame.setRect(frame.left(),frame.top(), frame.width()-1, frame.height()-1);
			painter->drawRect(frame);
			painter->setPen(option.palette.highlightedText().color());
			painter->fillRect(boundingRect, brush);

		}else
			painter->setPen(option.palette.text().color());

		auto newText = fm.elidedText(fileInfo.baseName(),Qt::ElideLeft,boundingRect.width());
		painter->drawText(option.rect, flags, newText);

		qDebug()<<"Painted";
	}
}

