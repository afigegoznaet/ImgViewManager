#include "imgthumbnaildelegate.h"
#include "imglistview.h"
ImgThumbnailDelegate::ImgThumbnailDelegate(QHash<QString, QImage> &cache, QObject* parent)
	: QItemDelegate(parent), currentCache(cache), canDraw(true){

	flags = Qt::AlignHCenter | Qt::AlignBottom;
}

void ImgThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							  const QModelIndex &index) const{


	if(index.isValid()){

		//qDebug()<<"Paint";
		auto fileInfo = model->data(index);
		//qDebug()<<"Got info";
		auto fileName = fileInfo.toString();
		//qDebug()<<"Got name";
		QFontMetrics fm(option.font);
		auto boundingRect = fm.boundingRect(option.rect,flags,fileName);
		boundingRect.setLeft(option.rect.left());
		boundingRect.setRight(option.rect.right());

		if(option.state & QStyle::State_Selected){
			auto brush = option.palette.highlight();
			//painter->fillRect(option.rect, brush);

			painter->setRenderHint(QPainter::HighQualityAntialiasing);
			QPainterPath path;
			path.addRoundedRect(option.rect, 4, 4);
			painter->fillPath(path, brush);
			painter->setPen(option.palette.highlightedText().color());

		}else
			painter->setPen(option.palette.text().color());


		if(index.isValid() && canDraw ){
			auto pixIt = currentCache.constFind(fileName);
			if(pixIt != currentCache.constEnd()){
				QPixmap pm = QPixmap::fromImage( *pixIt );
				int hDelta(0), vDelta(0);
				//qDebug()<<"pm size: "<<pm.size();
				//qDebug()<<"rect size: "<<option.rect;
				//qDebug()<<"width: "<<pm.width()<<" "<<option.rect.width();
				//qDebug()<<"height: "<<(pm.height()<option.rect.height());
				if(pm.width()<option.rect.width())
					hDelta = (option.rect.width() - pm.width())/2;

				if(pm.height() +	boundingRect.height() < option.rect.height())
					vDelta = (option.rect.height() -
							  boundingRect.height() - pm.height())/2;


				//qDebug()<<"hDelta: "<<hDelta;
				//qDebug()<<"vDelta: "<<vDelta;
				if(index.isValid()){
					if(hDelta >0 && vDelta >0)
					painter->drawPixmap(option.rect.left() + hDelta,
										option.rect.top() + vDelta,
						pm.width(), pm.height(),pm);
					else if(hDelta >0)
						painter->drawPixmap(option.rect.left() + hDelta,
											option.rect.top() + vDelta,
							pm.width(), option.rect.height()-boundingRect.height(),pm);

					else if(vDelta >0)
						painter->drawPixmap(option.rect.left() + hDelta,
											option.rect.top() + vDelta,
							option.rect.width() + hDelta, pm.height(),pm);
					else
						painter->drawPixmap(option.rect.left() + hDelta,
											option.rect.top() + vDelta,
							option.rect.width() + hDelta, option.rect.height()-boundingRect.height(),pm);
				}
			}else{
				painter->setRenderHint(QPainter::HighQualityAntialiasing);
				QPainterPath path;
				auto rectSize = qobject_cast<ImgListView*>(parent())->iconSize();
				QPoint pt = option.rect.topLeft();
				pt.setX(pt.x() + (option.rect.width() - rectSize.width())/2);
				QRect iconRect(pt, rectSize);
				path.addRoundedRect(iconRect, 4, 4);
				painter->setPen(Qt::lightGray);
				painter->drawPath(path);
				painter->setPen(option.palette.text().color());
			}

		}

		auto newText = fm.elidedText(fileInfo.toString().split('/').last(),Qt::ElideRight,boundingRect.width());
		painter->drawText(option.rect, flags, newText);
		//qDebug()<<"Painted";
	}
}

