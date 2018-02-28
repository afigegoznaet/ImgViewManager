#include "imgthumbnaildelegate.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QMap<QString, QPixmap> &cache, QObject* parent)
	: QItemDelegate(parent), currentCache(cache){
	QPixmapCache::setCacheLimit(100*QPixmapCache::cacheLimit());
	flags = Qt::AlignHCenter | Qt::AlignBottom;
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	if(index.isValid()){

		auto fileInfo = model->fileInfo(index);
		auto fileName = fileInfo.fileName();

		QFontMetrics fm(option.font);
		auto boundingRect = fm.boundingRect(option.rect,flags,fileName);

		auto pixIt = currentCache.constFind(fileName);
		if(!isExiting && pixIt != currentCache.constEnd()){
			QPixmap pm = *pixIt;
			painter->drawPixmap(option.rect.left()+1, option.rect.top()+1, pm);
		}

		if(option.state & QStyle::State_Selected){
			auto brush = option.palette.highlight();
			painter->setPen(option.palette.highlightedText().color());
			painter->fillRect(boundingRect, brush);
		}else
			painter->setPen(option.palette.text().color());

		painter->drawText(option.rect, flags, fileInfo.baseName());

	}
}

