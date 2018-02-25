#include "imgthumbnaildelegate.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QMap<QString, QPixmap> &cache, QObject* parent)
	: QItemDelegate(parent), currentCache(cache){
	QPixmapCache::setCacheLimit(100*QPixmapCache::cacheLimit());
	flags = Qt::AlignHCenter | Qt::AlignBottom
			| Qt::TextWrapAnywhere;
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	if(index.isValid()){

		if(isExiting)
			return;
		auto fileInfo = model->fileInfo(index);

		QFontMetrics fm(option.font);
		auto boundingRect = fm.boundingRect(option.rect,flags,fileInfo.fileName());

		auto fileName = fileInfo.fileName();
		if(currentCache.contains(fileName)){
			QPixmap pm = currentCache[fileInfo.fileName()];
			painter->drawPixmap(option.rect.left()+1, option.rect.top()+1, pm);
		}

		if(option.state & QStyle::State_Selected){
			qDebug()<<"Highlighted";
			auto brush = option.palette.highlight();
			painter->setPen(option.palette.highlightedText().color());
			painter->fillRect(boundingRect, brush);
		}else
			painter->setPen(option.palette.text().color());

		painter->drawText(option.rect, flags, fileInfo.fileName());

	}
}

