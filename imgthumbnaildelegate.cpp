#include "imgthumbnaildelegate.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QObject* parent)
	: QItemDelegate(parent){
	QPixmapCache::setCacheLimit(100*QPixmapCache::cacheLimit());
	flags = Qt::AlignHCenter | Qt::AlignBottom
			| Qt::TextWrapAnywhere;
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	if(index.isValid()){

		auto fileInfo = model->fileInfo(index);

		QFontMetrics fm(option.font);
		auto boundingRect = fm.boundingRect(option.rect,flags,fileInfo.fileName());

		QPixmap pm;
		if (QPixmapCache::find(fileInfo.absoluteFilePath(), &pm))
			painter->drawPixmap(option.rect.left()+1, option.rect.top()+1, pm);

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

