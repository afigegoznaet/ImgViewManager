#include "imgthumbnaildelegate.h"

ImgThumbnailDelegate::ImgThumbnailDelegate(QHash<QString, QPixmap> &cache, QObject* parent)
	: QItemDelegate(parent), currentCache(cache), canDraw(true){
	QPixmapCache::setCacheLimit(100*QPixmapCache::cacheLimit());
	flags = Qt::AlignHCenter | Qt::AlignBottom;
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{

	if(canDraw){
		//qDebug()<<"Paint";
		auto fileInfo = model->fileInfo(index);
		//qDebug()<<"Got info";
		auto fileName = fileInfo.fileName();
		//qDebug()<<"Got name";
		QFontMetrics fm(option.font);
		auto boundingRect = fm.boundingRect(option.rect,flags,fileName);
		boundingRect.setLeft(option.rect.left());
		boundingRect.setRight(option.rect.right());

		if(option.state & QStyle::State_Selected){
			auto brush = option.palette.highlight();
			painter->fillRect(option.rect, brush);
			/*QRect frame(option.rect);
			frame.setRect(frame.left()+1,frame.top()+1, frame.width()-1, frame.height()-1);
			painter->drawRect(option.rect);
			painter->setPen(brush.color());*/
			painter->setPen(option.palette.highlightedText().color());
			//painter->fillRect(boundingRect, brush);
		}else
			painter->setPen(option.palette.text().color());

		auto pixIt = currentCache.constFind(fileName);
		if(canDraw && pixIt != currentCache.constEnd()){
			QPixmap pm = *pixIt;
			int hDelta(0), vDelta(0);
			//qDebug()<<"pm size: "<<pm.size();
			//qDebug()<<"rect size: "<<option.rect;
			//qDebug()<<"width: "<<pm.width()<<" "<<option.rect.width();
			//qDebug()<<"height: "<<(pm.height()<option.rect.height());
			if(pm.width()<option.rect.width()
				|| pm.height()+
					boundingRect.height() < option.rect.height()){
				hDelta = (option.rect.width() - pm.width())/2;
				vDelta = (option.rect.height() -
						  boundingRect.height() - pm.height())/2;
			}

			//qDebug()<<"hDelta: "<<hDelta;
			//qDebug()<<"vDelta: "<<vDelta;
			if(hDelta && vDelta)
			painter->drawPixmap(option.rect.left() + hDelta,
								option.rect.top() + vDelta,
				pm.width(), pm.height(),pm);
			else if(hDelta )
				painter->drawPixmap(option.rect.left() + hDelta,
									option.rect.top() + vDelta,
					pm.width(), option.rect.height()-boundingRect.height(),pm);

			else if(vDelta )
				painter->drawPixmap(option.rect.left() + hDelta,
									option.rect.top() + vDelta,
					option.rect.width() + hDelta, pm.height(),pm);
			else
				painter->drawPixmap(option.rect.left() + hDelta,
									option.rect.top() + vDelta,
					option.rect.width() + hDelta, option.rect.height()-boundingRect.height(),pm);
		}


		auto newText = fm.elidedText(fileInfo.baseName(),Qt::ElideLeft,boundingRect.width());
		painter->drawText(option.rect, flags, newText);
		//qDebug()<<"Painted";
	}
}

